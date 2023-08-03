#include "pch.h"
#include "ScriptEngine.h"

#include <iostream>
#include <chrono>
#include <thread>

#include <winioctl.h>

#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/attrdefs.h>

#include"Engine/Scene/Scene.h"
#include"Engine/Script/ScriptEngineRegistry.h"

#include <imgui.h>

namespace Engine
{
	static MonoDomain* s_MonoDomain = nullptr;
	static std::string s_AssemblyPath;
	static Ref<Scene> s_SceneContext;

	// Assembly images
	MonoImage* s_AppAssemblyImage = nullptr;
	MonoImage* s_CoreAssemblyImage = nullptr;

	static MonoAssembly* s_AppAssembly = nullptr;
	static MonoAssembly* s_CoreAssembly = nullptr;

	static EntityInstanceMap s_EntityInstanceMap;


	static FieldType GetFieldType(MonoType* monoType)
	{
		int type = mono_type_get_type(monoType);
		switch (type)
		{
		case MONO_TYPE_R4: return FieldType::Float;
		case MONO_TYPE_I4: return FieldType::Int;
		case MONO_TYPE_U4: return FieldType::UnsignedInt;
		case MONO_TYPE_STRING: return FieldType::String;
		case MONO_TYPE_VALUETYPE:
		{
			char* name = mono_type_get_name(monoType);
			if (strcmp(name, "Engine.Vector2") == 0) return FieldType::Vec2;
			if (strcmp(name, "Engine.Vector3") == 0) return FieldType::Vec3;
			if (strcmp(name, "Engine.Vector4") == 0) return FieldType::Vec4;
		}
		}
		return FieldType::None;
	}

	const char* FieldTypeToString(FieldType type)
	{
		switch (type)
		{
		case FieldType::Float:       return "Float";
		case FieldType::Int:         return "Int";
		case FieldType::UnsignedInt: return "UnsignedInt";
		case FieldType::String:      return "String";
		case FieldType::Vec2:        return "Vec2";
		case FieldType::Vec3:        return "Vec3";
		case FieldType::Vec4:        return "Vec4";
		}
		return "Unknown";
	}


	static MonoMethod* GetMethod(MonoImage* image, const std::string& methodDesc);

	/// <summary>
	/// C#�ű��е����ͳ��󣬰�����Ļ����������������
	/// </summary>
	struct EntityScriptClass
	{
		std::string FullName;
		std::string ClassName;
		std::string NamespaceName;

		MonoClass* Class = nullptr;
		
		// Basic funtions
		MonoMethod* Constructor = nullptr;
		MonoMethod* OnCreateMethod = nullptr;
		MonoMethod* OnDestroyMethod = nullptr;
		MonoMethod* OnUpdateMethod = nullptr;
		MonoMethod* OnPhysicsUpdateMethod = nullptr;

		// Physics
		MonoMethod* OnCollisionBeginMethod = nullptr;
		MonoMethod* OnCollisionEndMethod = nullptr;
		MonoMethod* OnTriggerBeginMethod = nullptr;
		MonoMethod* OnTriggerEndMethod = nullptr;

		void InitClassMethods(MonoImage* image)
		{
			Constructor = GetMethod(s_CoreAssemblyImage, "Engine.Entity:.ctor(ulong)");
			OnCreateMethod = GetMethod(image, FullName + ":OnCreate()");
			OnUpdateMethod = GetMethod(image, FullName + ":OnUpdate(single)");
			OnPhysicsUpdateMethod = GetMethod(image, FullName + ":OnPhysicsUpdate(single)");

			// Physics (Entity class)
			OnCollisionBeginMethod = GetMethod(s_CoreAssemblyImage, "Engine.Entity:OnCollisionBegin(single)");
			OnCollisionEndMethod = GetMethod(s_CoreAssemblyImage, "Engine.Entity:OnCollisionEnd(single)");
			OnTriggerBeginMethod = GetMethod(s_CoreAssemblyImage, "Engine.Entity:OnTriggerBegin(single)");
			OnTriggerEndMethod = GetMethod(s_CoreAssemblyImage, "Engine.Entity:OnTriggerEnd(single)");
		}
	};

	MonoObject* EntityInstance::GetInstance()
	{
		ENGINE_ASSERT(Handle, "Entity has not been instantiated!");
		return mono_gchandle_get_target(Handle);
	}

	static std::unordered_map<std::string, EntityScriptClass> s_EntityClassMap;


	//---------------------------------------------------------------------------------
	// Functions
	//---------------------------------------------------------------------------------

	MonoAssembly* LoadAssemblyFromFile(const char* filepath)
	{
		if (filepath == NULL)
		{
			return NULL;
		}

		HANDLE file = CreateFileA(filepath, FILE_READ_ACCESS, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (file == INVALID_HANDLE_VALUE)
		{
			return NULL;
		}

		DWORD file_size = GetFileSize(file, NULL);
		if (file_size == INVALID_FILE_SIZE)
		{
			CloseHandle(file);
			return NULL;
		}

		void* file_data = malloc(file_size);
		if (file_data == NULL)
		{
			CloseHandle(file);
			return NULL;
		}

		DWORD read = 0;
		ReadFile(file, file_data, file_size, &read, NULL);
		if (file_size != read)
		{
			free(file_data);
			CloseHandle(file);
			return NULL;
		}

		MonoImageOpenStatus status;
		MonoImage* image = mono_image_open_from_data_full(reinterpret_cast<char*>(file_data), file_size, 1, &status, 0);
		if (status != MONO_IMAGE_OK)
		{
			return NULL;
		}
		auto assemb = mono_assembly_load_from_full(image, filepath, &status, 0);
		free(file_data);
		CloseHandle(file);
		mono_image_close(image);
		return assemb;
	}

	static void InitMono()
	{
		mono_set_assemblies_path("mono/lib");
		// mono_jit_set_trace_options("--verbose");
		auto domain = mono_jit_init("Engine");

		char* name = (char*)"EngineRuntime";
		s_MonoDomain = mono_domain_create_appdomain(name, nullptr);
	}

	static void ShutdownMono()
	{
		mono_jit_cleanup(s_MonoDomain);
	}

	static MonoAssembly* LoadAssembly(const std::string& path)
	{
		MonoAssembly* assembly = LoadAssemblyFromFile(path.c_str());

		if (!assembly)
			ENGINE_WARN("Could not load assembly: {0}", path);
		else
			ENGINE_INFO("Successfully loaded assembly: {0}", path);

		return assembly;
	}

	static MonoImage* GetAssemblyImage(MonoAssembly* assembly)
	{
		MonoImage* image = mono_assembly_get_image(assembly);
		if (!image)
			ENGINE_WARN("mono_assembly_get_image failed");

		return image;
	}

	static MonoClass* GetClass(MonoImage* image, const EntityScriptClass& scriptClass)
	{
		MonoClass* monoClass = mono_class_from_name(image, scriptClass.NamespaceName.c_str(), scriptClass.ClassName.c_str());
		if (!monoClass)
			ENGINE_WARN("mono_class_from_name failed");

		return monoClass;
	}

	static uint32_t Instantiate(EntityScriptClass& scriptClass)
	{
		MonoObject* instance = mono_object_new(s_MonoDomain, scriptClass.Class);
		if (!instance)
			ENGINE_WARN("mono_object_new failed");

		mono_runtime_object_init(instance);
		uint32_t handle = mono_gchandle_new(instance, false);
		return handle;
	}

	static MonoMethod* GetMethod(MonoImage* image, const std::string& methodDesc)
	{
		MonoMethodDesc* desc = mono_method_desc_new(methodDesc.c_str(), NULL);
		if (!desc)
			ENGINE_WARN("mono_method_desc_new failed");

		MonoMethod* method = mono_method_desc_search_in_image(desc, image);
		if (!method)
			ENGINE_WARN("mono_method_desc_search_in_image failed");

		return method;
	}

	static MonoObject* CallMethod(MonoObject* object, MonoMethod* method, void** params = nullptr)
	{
		MonoObject* pException = NULL;
		MonoObject* result = mono_runtime_invoke(method, object, params, &pException);
		return result;
	}

	static void PrintClassMethods(MonoClass* monoClass)
	{
		MonoMethod* iter;
		void* ptr = 0;
		while ((iter = mono_class_get_methods(monoClass, &ptr)) != NULL)
		{
			printf("--------------------------------\n");
			const char* name = mono_method_get_name(iter);
			MonoMethodDesc* methodDesc = mono_method_desc_from_method(iter);

			const char* paramNames = "";
			mono_method_get_param_names(iter, &paramNames);

			printf("Name: %s\n", name);
			printf("Full name: %s\n", mono_method_full_name(iter, true));
		}
	}

	static void PrintClassProperties(MonoClass* monoClass)
	{
		MonoProperty* iter;
		void* ptr = 0;
		while ((iter = mono_class_get_properties(monoClass, &ptr)) != NULL)
		{
			printf("--------------------------------\n");
			const char* name = mono_property_get_name(iter);

			printf("Name: %s\n", name);
		}
	}

	static MonoString* GetName()
	{
		return mono_string_new(s_MonoDomain, "Hello!");
	}

	//---------------------------------------------------------------------------------
	//ScriptEngine
	//---------------------------------------------------------------------------------
	void ScriptEngine::Init(const std::string& assemblyPath)
	{
		s_AssemblyPath = assemblyPath;

		InitMono();

		LoadRuntimeAssembly(s_AssemblyPath);
	}

	void ScriptEngine::Shutdown()
	{
		s_SceneContext = nullptr;
		s_EntityInstanceMap.clear();
	}

	void ScriptEngine::OnSceneDestruct(UUID sceneID)
	{
		if (s_EntityInstanceMap.find(sceneID) != s_EntityInstanceMap.end())
		{
			s_EntityInstanceMap.at(sceneID).clear();
			s_EntityInstanceMap.erase(sceneID);
		}
	}

	void ScriptEngine::LoadRuntimeAssembly(const std::string& path)
	{
		MonoDomain* domain = nullptr;
		bool cleanup = false;
		if (s_MonoDomain)
		{
			domain = mono_domain_create_appdomain("Engine Runtime", nullptr);
			mono_domain_set(domain, false);

			cleanup = true;
		}

		s_CoreAssembly = LoadAssembly("assets/scripts/Engine-ScriptCore.dll");
		s_CoreAssemblyImage = GetAssemblyImage(s_CoreAssembly);

		auto appAssembly = LoadAssembly(path);
		auto appAssemblyImage = GetAssemblyImage(appAssembly);
		ScriptEngineRegistry::RegisterAll();

		if (cleanup)
		{
			mono_domain_unload(s_MonoDomain);
			s_MonoDomain = domain;
		}

		s_AppAssembly = appAssembly;
		s_AppAssemblyImage = appAssemblyImage;
	}

	void ScriptEngine::ReloadAssembly(const std::string& path)
	{
		LoadRuntimeAssembly(path);
		if (s_EntityInstanceMap.size())
		{
			Ref<Scene> scene = ScriptEngine::GetCurrentSceneContext();
			ENGINE_ASSERT(scene, "No active scene!");
			if (s_EntityInstanceMap.find(scene->GetUUID()) != s_EntityInstanceMap.end())
			{
				auto& entityMap = s_EntityInstanceMap.at(scene->GetUUID());
				for (auto& [entityID, entityInstanceData] : entityMap)
				{
					const auto& entityMap = scene->GetEntityMap();
					ENGINE_ASSERT(entityMap.find(entityID) != entityMap.end(), "Invalid entity ID or entity doesn't exist in scene!");
					InitScriptEntity(entityMap.at(entityID));
				}
			}
		}
	}

	void ScriptEngine::SetSceneContext(const Ref<Scene>& scene)
	{
		s_SceneContext = scene;
	}

	const Ref<Scene>& ScriptEngine::GetCurrentSceneContext()
	{
		return s_SceneContext;
	}

	void ScriptEngine::CopyEntityScriptData(UUID dst, UUID src)
	{
		ENGINE_ASSERT(s_EntityInstanceMap.find(dst) != s_EntityInstanceMap.end(),"");
		ENGINE_ASSERT(s_EntityInstanceMap.find(src) != s_EntityInstanceMap.end(),"");

		auto& dstEntityMap = s_EntityInstanceMap.at(dst);
		auto& srcEntityMap = s_EntityInstanceMap.at(src);

		for (auto& [entityID, entityInstanceData] : srcEntityMap)
		{
			for (auto& [moduleName, srcFieldMap] : srcEntityMap[entityID].ModuleFieldMap)
			{
				auto& dstModuleFieldMap = dstEntityMap[entityID].ModuleFieldMap;
				for (auto& [fieldName, field] : srcFieldMap)
				{
					ENGINE_ASSERT(dstModuleFieldMap.find(moduleName) != dstModuleFieldMap.end(),"");
					auto& fieldMap = dstModuleFieldMap.at(moduleName);
					ENGINE_ASSERT(fieldMap.find(fieldName) != fieldMap.end(),"");
					fieldMap.at(fieldName).SetStoredValueRaw(field.m_StoredValueBuffer);
				}
			}
		}
	}

	void ScriptEngine::OnCreateEntity(Entity entity)
	{
		EntityInstance& entityInstance = GetEntityInstanceData(entity.GetSceneUUID(), entity.ID()).Instance;
		if (entityInstance.ScriptClass->OnCreateMethod)
			CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->OnCreateMethod);
	}

	void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
	{
		EntityInstance& entityInstance = GetEntityInstanceData(entity.GetSceneUUID(), entity.ID()).Instance;
		if (entityInstance.ScriptClass->OnUpdateMethod)
		{
			void* args[] = { &ts };
			CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->OnUpdateMethod, args);
		}
	}

	void ScriptEngine::OnPhysicsUpdateEntity(Entity entity, float fixedTimeStep)
	{
		EntityInstance& entityInstance = GetEntityInstanceData(entity.GetSceneUUID(), entity.ID()).Instance;
		if (entityInstance.ScriptClass->OnPhysicsUpdateMethod)
		{
			void* args[] = { &fixedTimeStep };
			CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->OnPhysicsUpdateMethod, args);
		}
	}

	void ScriptEngine::OnCollisionBegin(Entity entity)
	{
		EntityInstance& entityInstance = GetEntityInstanceData(entity.GetSceneUUID(), entity.ID()).Instance;
		if (entityInstance.ScriptClass->OnCollisionBeginMethod)
		{
			float value = 5.0f;
			void* args[] = { &value };
			CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->OnCollisionBeginMethod, args);
		}
	}

	void ScriptEngine::OnCollisionEnd(Entity entity)
	{
		EntityInstance& entityInstance = GetEntityInstanceData(entity.GetSceneUUID(), entity.ID()).Instance;
		if (entityInstance.ScriptClass->OnCollisionEndMethod)
		{
			float value = 5.0f;
			void* args[] = { &value };
			CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->OnCollisionEndMethod, args);
		}
	}

	void ScriptEngine::OnTriggerBegin(Entity entity)
	{
		EntityInstance& entityInstance = GetEntityInstanceData(entity.GetSceneUUID(), entity.ID()).Instance;
		if (entityInstance.ScriptClass->OnTriggerBeginMethod)
		{
			float value = 5.0f;
			void* args[] = { &value };
			CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->OnTriggerBeginMethod, args);
		}
	}

	void ScriptEngine::OnTriggerEnd(Entity entity)
	{
		EntityInstance& entityInstance = GetEntityInstanceData(entity.GetSceneUUID(), entity.ID()).Instance;
		if (entityInstance.ScriptClass->OnTriggerEndMethod)
		{
			float value = 5.0f;
			void* args[] = { &value };
			CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->OnTriggerEndMethod, args);
		}
	}

	MonoObject* ScriptEngine::Construct(const std::string& fullName, bool callConstructor, void** parameters)
	{
		std::string namespaceName;
		std::string className;
		std::string parameterList;

		if (fullName.find(".") != std::string::npos)
		{
			namespaceName = fullName.substr(0, fullName.find_first_of('.'));
			className = fullName.substr(fullName.find_first_of('.') + 1, (fullName.find_first_of(':') - fullName.find_first_of('.')) - 1);
		}

		if (fullName.find(":") != std::string::npos)
		{
			parameterList = fullName.substr(fullName.find_first_of(':'));
		}

		MonoClass* clazz = mono_class_from_name(s_CoreAssemblyImage, namespaceName.c_str(), className.c_str());
		MonoObject* obj = mono_object_new(mono_domain_get(), clazz);

		if (callConstructor)
		{
			MonoMethodDesc* desc = mono_method_desc_new(parameterList.c_str(), NULL);
			MonoMethod* constructor = mono_method_desc_search_in_class(desc, clazz);
			MonoObject* exception = nullptr;
			mono_runtime_invoke(constructor, obj, parameters, &exception);
		}

		return obj;
	}

	static std::unordered_map<std::string, MonoClass*> s_Classes;

	MonoClass* ScriptEngine::GetCoreClass(const std::string& fullName)
	{
		if (s_Classes.find(fullName) != s_Classes.end())
			return s_Classes[fullName];

		std::string namespaceName = "";
		std::string className;

		if (fullName.find('.') != std::string::npos)
		{
			namespaceName = fullName.substr(0, fullName.find_last_of('.'));
			className = fullName.substr(fullName.find_last_of('.') + 1);
		}
		else
		{
			className = fullName;
		}

		MonoClass* monoClass = mono_class_from_name(s_CoreAssemblyImage, namespaceName.c_str(), className.c_str());
		if (!monoClass)
			ENGINE_WARN("mono_class_from_name failed");

		s_Classes[fullName] = monoClass;

		return monoClass;
	}

	bool ScriptEngine::IsEntityModuleValid(Entity entity)
	{
		return entity.HasComponent<ScriptComponent>() && ModuleExists(entity.GetComponent<ScriptComponent>().ModuleName);
	}

	void ScriptEngine::OnScriptComponentDestroyed(UUID sceneID, UUID entityID)
	{
		ENGINE_ASSERT(s_EntityInstanceMap.find(sceneID) != s_EntityInstanceMap.end(),"");
		auto& entityMap = s_EntityInstanceMap.at(sceneID);
		ENGINE_ASSERT(entityMap.find(entityID) != entityMap.end(),"");
		entityMap.erase(entityID);
	}

	bool ScriptEngine::ModuleExists(const std::string& moduleName)
	{
		std::string NamespaceName, ClassName;
		if (moduleName.find('.') != std::string::npos)
		{
			NamespaceName = moduleName.substr(0, moduleName.find_last_of('.'));
			ClassName = moduleName.substr(moduleName.find_last_of('.') + 1);
		}
		else
		{
			ClassName = moduleName;
		}

		MonoClass* monoClass = mono_class_from_name(s_AppAssemblyImage, NamespaceName.c_str(), ClassName.c_str());
		return monoClass != nullptr;
	}

	void ScriptEngine::InitScriptEntity(Entity entity)
	{
		Scene* scene = entity.m_Scene;
		UUID id = entity.GetComponent<IDComponent>().ID;
		auto& moduleName = entity.GetComponent<ScriptComponent>().ModuleName;
		if (moduleName.empty())
			return;

		if (!ModuleExists(moduleName))
		{
			ENGINE_ERROR("Entity references non-existent script module '{0}'", moduleName);
			return;
		}

		EntityScriptClass& scriptClass = s_EntityClassMap[moduleName];
		scriptClass.FullName = moduleName;
		if (moduleName.find('.') != std::string::npos)
		{
			scriptClass.NamespaceName = moduleName.substr(0, moduleName.find_last_of('.'));
			scriptClass.ClassName = moduleName.substr(moduleName.find_last_of('.') + 1);
		}
		else
		{
			scriptClass.ClassName = moduleName;
		}

		scriptClass.Class = GetClass(s_AppAssemblyImage, scriptClass);
		scriptClass.InitClassMethods(s_AppAssemblyImage);

		EntityInstanceData& entityInstanceData = s_EntityInstanceMap[scene->GetUUID()][id];
		EntityInstance& entityInstance = entityInstanceData.Instance;
		entityInstance.ScriptClass = &scriptClass;
		ScriptModuleFieldMap& moduleFieldMap = entityInstanceData.ModuleFieldMap;
		auto& fieldMap = moduleFieldMap[moduleName];

		// Save old fields
		std::unordered_map<std::string, PublicField> oldFields;
		oldFields.reserve(fieldMap.size());
		for (auto& [fieldName, field] : fieldMap)
			oldFields.emplace(fieldName, std::move(field));
		fieldMap.clear();

		// Retrieve public fields (TODO: cache these fields if the module is used more than once)
		{
			MonoClassField* iter;
			void* ptr = 0;
			while ((iter = mono_class_get_fields(scriptClass.Class, &ptr)) != NULL)
			{
				const char* name = mono_field_get_name(iter);
				uint32_t flags = mono_field_get_flags(iter);
				if ((flags & MONO_FIELD_ATTR_PUBLIC) == 0)
					continue;

				MonoType* fieldType = mono_field_get_type(iter);
				FieldType hazelFieldType = GetFieldType(fieldType);

				// TODO: Attributes
				MonoCustomAttrInfo* attr = mono_custom_attrs_from_field(scriptClass.Class, iter);

				if (oldFields.find(name) != oldFields.end())
				{
					fieldMap.emplace(name, std::move(oldFields.at(name)));
				}
				else
				{
					PublicField field = { name, hazelFieldType };
					field.m_EntityInstance = &entityInstance;
					field.m_MonoClassField = iter;
					fieldMap.emplace(name, std::move(field));
				}
			}
		}
	}

	void ScriptEngine::ShutdownScriptEntity(Entity entity, const std::string& moduleName)
	{
		EntityInstanceData& entityInstanceData = GetEntityInstanceData(entity.GetSceneUUID(), entity.ID());
		ScriptModuleFieldMap& moduleFieldMap = entityInstanceData.ModuleFieldMap;
		if (moduleFieldMap.find(moduleName) != moduleFieldMap.end())
			moduleFieldMap.erase(moduleName);
	}

	void ScriptEngine::InstantiateEntityClass(Entity entity)
	{
		Scene* scene = entity.m_Scene;
		UUID id = entity.GetComponent<IDComponent>().ID;
		auto& moduleName = entity.GetComponent<ScriptComponent>().ModuleName;

		EntityInstanceData& entityInstanceData = GetEntityInstanceData(scene->GetUUID(), id);
		EntityInstance& entityInstance = entityInstanceData.Instance;
		ENGINE_ASSERT(entityInstance.ScriptClass,"");
		entityInstance.Handle = Instantiate(*entityInstance.ScriptClass);

		void* param[] = { &id };
		CallMethod(entityInstance.GetInstance(), entityInstance.ScriptClass->Constructor, param);

		// Set all public fields to appropriate values
		ScriptModuleFieldMap& moduleFieldMap = entityInstanceData.ModuleFieldMap;
		if (moduleFieldMap.find(moduleName) != moduleFieldMap.end())
		{
			auto& publicFields = moduleFieldMap.at(moduleName);
			for (auto& [name, field] : publicFields)
				field.CopyStoredValueToRuntime();
		}

		// Call OnCreate function (if exists)
		OnCreateEntity(entity);
	}

	EntityInstanceMap& ScriptEngine::GetEntityInstanceMap()
	{
		return s_EntityInstanceMap;
	}

	EntityInstanceData& ScriptEngine::GetEntityInstanceData(UUID sceneID, UUID entityID)
	{
		ENGINE_ASSERT(s_EntityInstanceMap.find(sceneID) != s_EntityInstanceMap.end(), "Invalid scene ID!");
		auto& entityIDMap = s_EntityInstanceMap.at(sceneID);
		ENGINE_ASSERT(entityIDMap.find(entityID) != entityIDMap.end(), "Invalid entity ID!");
		return entityIDMap.at(entityID);
	}

	void ScriptEngine::OnImGuiRender()
	{
		// Debug
		ImGui::Begin("Script Debug");
		for (auto& [sceneID, entityMap] : s_EntityInstanceMap)
		{
			bool opened = ImGui::TreeNode((void*)(uint64_t)sceneID, "Scene (%llx)", sceneID);
			if (opened)
			{
				auto scene = Scene::GetScene(sceneID);

				for (auto& [entityID, entityInstanceData] : entityMap)
				{
					Entity entity = scene->GetEntityMap().at(entityID);
					std::string entityName = "Unnamed Entity";
					if (entity.HasComponent<TagComponent>())
						entityName = entity.GetComponent<TagComponent>().Tag;
					opened = ImGui::TreeNode((void*)(uint64_t)entityID, "%s (%llx)", entityName.c_str(), entityID);
					if (opened)
					{
						for (auto& [moduleName, fieldMap] : entityInstanceData.ModuleFieldMap)
						{
							opened = ImGui::TreeNode(moduleName.c_str());
							if (opened)
							{
								for (auto& [fieldName, field] : fieldMap)
								{

									opened = ImGui::TreeNodeEx((void*)&field, ImGuiTreeNodeFlags_Leaf, fieldName.c_str());
									if (opened)
									{

										ImGui::TreePop();
									}
								}
								ImGui::TreePop();
							}
						}
						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}
		}
		ImGui::End();
	}

	//---------------------------------------------------------------------------------
	//PublicField
	//---------------------------------------------------------------------------------
	
	static uint32_t GetFieldSize(FieldType type)
	{
		switch (type)
		{
		case FieldType::Float:       return 4;
		case FieldType::Int:         return 4;
		case FieldType::UnsignedInt: return 4;
			// case FieldType::String:   return 8; // TODO
		case FieldType::Vec2:        return 4 * 2;
		case FieldType::Vec3:        return 4 * 3;
		case FieldType::Vec4:        return 4 * 4;
		}
		ENGINE_ASSERT(false, "Unknown field type!");
		return 0;
	}
	
	PublicField::PublicField(const std::string& name, FieldType type)
		: Name(name), Type(type)
	{
		m_StoredValueBuffer = AllocateBuffer(type);
	}

	PublicField::PublicField(PublicField&& other)
	{
		Name = std::move(other.Name);
		Type = other.Type;
		m_EntityInstance = other.m_EntityInstance;
		m_MonoClassField = other.m_MonoClassField;
		m_StoredValueBuffer = other.m_StoredValueBuffer;

		other.m_EntityInstance = nullptr;
		other.m_MonoClassField = nullptr;
		other.m_StoredValueBuffer = nullptr;
	}

	PublicField::~PublicField()
	{
		delete[] m_StoredValueBuffer;
	}

	void PublicField::CopyStoredValueToRuntime()
	{
		ENGINE_ASSERT(m_EntityInstance->GetInstance(),"");
		mono_field_set_value(m_EntityInstance->GetInstance(), m_MonoClassField, m_StoredValueBuffer);
	}

	bool PublicField::IsRuntimeAvailable() const
	{
		return m_EntityInstance->Handle != 0;
	}

	void PublicField::SetStoredValueRaw(void* src)
	{
		uint32_t size = GetFieldSize(Type);
		memcpy(m_StoredValueBuffer, src, size);
	}

	uint8_t* PublicField::AllocateBuffer(FieldType type)
	{
		uint32_t size = GetFieldSize(type);
		uint8_t* buffer = new uint8_t[size];
		memset(buffer, 0, size);
		return buffer;
	}

	void PublicField::SetStoredValue_Internal(void* value) const
	{
		uint32_t size = GetFieldSize(Type);
		memcpy(m_StoredValueBuffer, value, size);
	}

	void PublicField::GetStoredValue_Internal(void* outValue) const
	{
		uint32_t size = GetFieldSize(Type);
		memcpy(outValue, m_StoredValueBuffer, size);
	}

	void PublicField::SetRuntimeValue_Internal(void* value) const
	{
		ENGINE_ASSERT(m_EntityInstance->GetInstance(),"");
		mono_field_set_value(m_EntityInstance->GetInstance(), m_MonoClassField, value);
	}

	void PublicField::GetRuntimeValue_Internal(void* outValue) const
	{
		ENGINE_ASSERT(m_EntityInstance->GetInstance(),"");
		mono_field_get_value(m_EntityInstance->GetInstance(), m_MonoClassField, outValue);
	}
}
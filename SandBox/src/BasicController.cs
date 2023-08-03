using Engine;
using src.Engine;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SandBox
{
    public class BasicController : Entity
    {
        public float Speed = 10.0f;

        private TransformComponent m_Transform;

        private float m_CurrentSpeed;
        private Vector2 m_MovementDirection = new Vector2(0.0f);

        public void OnCreate()
        {
            Console.WriteLine("Basic controller create");

            m_Transform = GetComponent<TransformComponent>();

            m_CurrentSpeed = Speed;
        }

        public void OnUpdate(float ts)
        {
            UpdateMovementInput();
            UpdateMovement(ts);

            if (Input.IsKeyPressed(KeyCode.Space))
                SetRandomColor();
        }


        private void SetRandomColor()
        {
            MeshComponent meshComponent = GetComponent<MeshComponent>();
            MaterialInstance material = meshComponent.Mesh.GetMaterial(0);
            Random random = new Random();
            float r = (float)random.NextDouble();
            float g = (float)random.NextDouble();
            float b = (float)random.NextDouble();
            material.Set("u_AlbedoColor", new Vector3(r, g, b));
        }

        public void OnPhysicsUpdate(float fixedTimeStep)
        {
            Console.WriteLine("Physics update");
            //UpdateMovement(fixedTimeStep);
        }

        public void OnDestroy()
        {
            Console.WriteLine("Basic controller destroy");
        }

        private void UpdateMovementInput()
        {
            if (Input.IsKeyPressed(KeyCode.W))
            {
                Console.WriteLine("Press W");
                m_MovementDirection.Y = 1.0F;
            }
            else if (Input.IsKeyPressed(KeyCode.S))
            {
                Console.WriteLine("Press S");
                m_MovementDirection.Y = -1.0F;
            }
            else
                m_MovementDirection.Y = 0.0F;

            if (Input.IsKeyPressed(KeyCode.A))
            {
                Console.WriteLine("Press A");
                m_MovementDirection.X = -1.0F;
            }
            else if (Input.IsKeyPressed(KeyCode.D))
            {
                Console.WriteLine("Press D");
                m_MovementDirection.X = 1.0F;
            }
            else
                m_MovementDirection.X = 0.0F;
        }
        private void UpdateMovement(float ts)
        {
            Vector3 movement = new Vector3(m_MovementDirection.X, 0.0f, m_MovementDirection.Y);
            
            m_Transform.Position += movement;
        }
    }
}

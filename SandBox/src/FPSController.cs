using Engine;
using src.Engine;
using src.Engine.Physics;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SandBox
{
    //Unimplemented
    public class FPSController : Entity
    {
        public float Speed = 10.0f;
        public float JumpForce = 25.0F;
        public float CameraForwardOffset = 0.2F;
        public float CameraYOffset = 0.85F;

        public float MouseSensitivity = 10.0F;

        private RigidBodyComponent m_RigidBody;
        private TransformComponent m_Transform;

        private Entity m_CameraEntity;
        private TransformComponent m_CameraTransform;

        private float m_CurrentSpeed;
        private Vector2 m_MovementDirection = new Vector2(0.0F);
        private bool m_ShouldJump = false;
        private float m_CurrentYMovement = 0.0F;
        private bool m_Colliding = false;

        private Vector2 m_LastMousePosition;

        public void OnCreate()
        {
            m_Transform = GetComponent<TransformComponent>();
            m_RigidBody = GetComponent<RigidBodyComponent>();

            m_CurrentSpeed = Speed;

            m_CameraEntity = FindEntityByTag("Camera");
            m_CameraTransform = m_CameraEntity.GetComponent<TransformComponent>();

            m_LastMousePosition = Input.GetMousePosition();
            
            //Input.SetCursorMode(CursorMode.Locked);

            AddCollisionBeginCallback((n) => { m_Colliding = true; });
            AddCollisionEndCallback((n) => { m_Colliding = false; });
        }

        public void OnUpdate(float ts)
        {
            /*
            if (Input.IsKeyPressed(KeyCode.Escape) && Input.GetCursorMode() == CursorMode.Locked)
                Input.SetCursorMode(CursorMode.Normal);
            if (Input.IsMouseButtonPressed(MouseButton.Left) && Input.GetCursorMode() == CursorMode.Normal)
                Input.SetCursorMode(CursorMode.Locked);
            */
            UpdateMovementInput();
            //UpdateRotation(ts);
            //UpdateCameraTransform();
        }

        public void OnPhysicsUpdate(float fixedTimeStep)
        {
            UpdateMovement();
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

            m_ShouldJump = Input.IsKeyPressed(KeyCode.Space) && !m_ShouldJump;
        }

        private void UpdateMovement()
        {
            m_RigidBody.Rotate(new Vector3(0.0F, m_CurrentYMovement, 0.0F));

            Vector3 movement = m_CameraTransform.Transform.Right * m_MovementDirection.X + m_CameraTransform.Transform.Forward * m_MovementDirection.Y;
            movement.Normalize();
            Vector3 velocity = movement * m_CurrentSpeed;
            velocity.Y = m_RigidBody.GetLinearVelocity().Y;
            m_RigidBody.SetLinearVelocity(velocity);

            if (m_ShouldJump && m_Colliding)
            {
                m_RigidBody.AddForce(Vector3.Up * JumpForce, ForceMode.Impulse);
                m_ShouldJump = false;
            }
        }

        private void UpdateRotation(float ts)
        {
            // TODO: Mouse position should be relative to the viewport
            Vector2 currentMousePosition = Input.GetMousePosition();
            Vector2 delta = m_LastMousePosition - currentMousePosition;
            m_CurrentYMovement = delta.X * MouseSensitivity * ts;
            float xRotation = delta.Y * MouseSensitivity * ts;

            if (delta.Y != 0.0F || delta.X != 0.0F)
            {
                m_CameraTransform.Rotation += new Vector3(xRotation, m_CurrentYMovement, 0.0F);
            }

            m_CameraTransform.Rotation = new Vector3(Mathf.Clamp(m_CameraTransform.Rotation.X, -80.0F, 80.0F), m_CameraTransform.Rotation.YZ);

            m_LastMousePosition = currentMousePosition;
        }

        private void UpdateCameraTransform()
        {
            Vector3 position = m_Transform.Position + m_Transform.Transform.Forward * CameraForwardOffset;
            position.Y = m_Transform.Position.Y + CameraYOffset;
            m_CameraTransform.Position = position;
        }
    }
}

#include "Player.h"
#include <Kernel/OVR_Alg.h>


Player::Player() :
    UserEyeHeight(1.76f - 0.15f), // 1.76 meters height (ave US male, Wikipedia), less 15 centimeters (TomF's top-of-head-to-eye distance).
    HeightScale(1.0f),
    BodyPos(7.7f, 1.76f - 0.15f, -1.0f),
    BodyYaw(YawInitial + 1.6f),	// so then we are facing the objects we create
    HeadPose(),
    MoveForward(0),
    MoveBack(0),
    MoveLeft(0),
    MoveRight(0),
    bMotionRelativeToBody(false)
{
}

Player::~Player()
{
}

Vector3f Player::GetPosition()
{
    return BodyPos + Quatf(Vector3f(0,1,0), BodyYaw.Get()).Rotate(HeadPose.Translation);
}

Quatf Player::GetOrientation(bool baseOnly)
{
    Quatf baseQ = Quatf(Vector3f(0,1,0), BodyYaw.Get());
    return baseOnly ? baseQ : baseQ * HeadPose.Rotation;
}

Posef Player::VirtualWorldTransformfromRealPose(const Posef &sensorHeadPose)
{
    Quatf baseQ = Quatf(Vector3f(0,1,0), BodyYaw.Get());

    return Posef(baseQ * sensorHeadPose.Rotation,
                 BodyPos + baseQ.Rotate(sensorHeadPose.Translation));
}

void Player::HandleMovement(double dt, Array<Ptr<CollisionModel> >* collisionModels,
	                        Array<Ptr<CollisionModel> >* groundCollisionModels, bool shiftDown)
{
    // Handle keyboard movement.
    // This translates BasePos based on the orientation and keys pressed.
    // Note that Pitch and Roll do not affect movement (they only affect view).
    Vector3f controllerMove;
    if(MoveForward || MoveBack || MoveLeft || MoveRight)
    {
        if (MoveForward)
        {
            controllerMove += ForwardVector;
        }
        else if (MoveBack)
        {
            controllerMove -= ForwardVector;
        }

        if (MoveRight)
        {
            controllerMove += RightVector;
        }
        else if (MoveLeft)
        {
            controllerMove -= RightVector;
        }
    }
    controllerMove = GetOrientation(bMotionRelativeToBody).Rotate(controllerMove);    
    controllerMove.y = 0; // Project to the horizontal plane
    if (controllerMove.LengthSq() > 0)
    {
        // Normalize vector so we don't move faster diagonally.
        controllerMove.Normalize();
        controllerMove *= OVR::Alg::Min<float>(MoveSpeed * (float)dt * (shiftDown ? 3.0f : 1.0f), 1.0f);
    }

    // Compute total move direction vector and move length
    Vector3f orientationVector = controllerMove;
    float moveLength = orientationVector.Length();
    if (moveLength > 0)
        orientationVector.Normalize();
        
    float   checkLengthForward = moveLength;

    orientationVector *= moveLength;
    BodyPos += orientationVector;

    float finalDistanceDown = GetScaledEyeHeight() + 10.0f;

}

// Handle directional movement. Returns 'true' if movement was processed.
bool Player::HandleMoveKey(OVR::KeyCode key, bool down)
{
    switch(key)
    {
        // Handle player movement keys.
        // We just update movement state here, while the actual translation is done in OnIdle()
        // based on time.
    case OVR::Key_W:     MoveForward = down ? (MoveForward | 1) : (MoveForward & ~1); return true;
    case OVR::Key_S:     MoveBack    = down ? (MoveBack    | 1) : (MoveBack    & ~1); return true;
    case OVR::Key_A:     MoveLeft    = down ? (MoveLeft    | 1) : (MoveLeft    & ~1); return true;
    case OVR::Key_D:     MoveRight   = down ? (MoveRight   | 1) : (MoveRight   & ~1); return true;
    case OVR::Key_Up:    MoveForward = down ? (MoveForward | 2) : (MoveForward & ~2); return true;
    case OVR::Key_Down:  MoveBack    = down ? (MoveBack    | 2) : (MoveBack    & ~2); return true;
    case OVR::Key_Left:  MoveLeft    = down ? (MoveLeft    | 2) : (MoveLeft    & ~2); return true;
    case OVR::Key_Right: MoveRight   = down ? (MoveRight   | 2) : (MoveRight   & ~2); return true;
    default: return false;
    }
}

//---------- INCLUDES ----------//

#include "SharedIncludes.h"



//---------- FUNCTIONS ----------//

/// @brief  Constructor
Input::Input (OIS::ParamList paramList)
{
    // Create input system.
    mInputManager = OIS::InputManager::createInputSystem(paramList);

    // Create keyboard and mouse inputs.
    mKeyboard = static_cast<OIS::Keyboard*>(mInputManager->createInputObject(OIS::OISKeyboard, true));
    mMouse    = static_cast<OIS::Mouse*>(   mInputManager->createInputObject(OIS::OISMouse,    true));
    
    mMouse->setEventCallback(this);
    mKeyboard->setEventCallback(this);
}



/// @brief  Deconstructor
Input::~Input (void)
{
    // Destroy input system.
    mInputManager->destroyInputObject(mMouse);
    mInputManager->destroyInputObject(mKeyboard);

    OIS::InputManager::destroyInputSystem(mInputManager);
    mInputManager = 0;
}



void Input::capture (void)
{
    mKeyboard->capture();
    mMouse->capture();
}



bool Input::keyPressed (const OIS::KeyEvent &arg)
{
    // Inject keypress into CEGUI.
    CEGUI::System &sys = CEGUI::System::getSingleton();
    sys.injectKeyDown(arg.key);
    sys.injectChar(arg.text);

    return true;
}



bool Input::keyReleased (const OIS::KeyEvent &arg)
{
    // Inject keyrelease into CEGUI.
    CEGUI::System::getSingleton().injectKeyUp(arg.key);

    return true;
}



bool Input::mouseMoved (const OIS::MouseEvent &arg)
{
    // Inject mouse into CEGUI.
    CEGUI::System &sys = CEGUI::System::getSingleton();
    sys.injectMouseMove(arg.state.X.rel, arg.state.Y.rel);
    if (arg.state.Z.rel)
        sys.injectMouseWheelChange(arg.state.Z.rel / 120.0f);

    // Process mouse movements.
    if (arg.state.buttonDown(OIS::MB_Left))
    {
        // If player is holding down LMB, update aim reticule.
        //mPlayerAimHeight += arg.state.Y.rel;
        //drawTargetPath(mPlayerAimHeight);
    }
    else if (arg.state.buttonDown(OIS::MB_Right))
    {
        // If player is holding down RMB, update player rotation.
        //mPlayerNode->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(Ogre::Degree(-0.3 * arg.state.X.rel)));
    }

    return true;
}



bool Input::mousePressed (const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
    // Inject mouse into CEGUI.
    CEGUI::System::getSingleton().injectMouseButtonDown(convertButton(id));

    // Process mouse presses.
    if (id == OIS::MB_Left)
    {
        OIS::MouseState ms = mMouse->getMouseState();
        Core::mPlayer->showTargettingSystem(ms);
    }

    return true;
}



bool Input::mouseReleased (const OIS::MouseEvent &arg, OIS::MouseButtonID id)
{
    // Inject mouse into CEGUI.
    CEGUI::System::getSingleton().injectMouseButtonUp(convertButton(id));

    // Process mouse releases.
    if (id == OIS::MB_Left)
    {
        // Hide the pathing nodule.
        Core::mPlayer->hideTargettingSystem();

        // Show the normal cursor again.
        CEGUI::MouseCursor::getSingleton().show();
    }

    return true;
}



/// @brief  Converts OIS buttons to CEGUI types.
CEGUI::MouseButton Input::convertButton (OIS::MouseButtonID buttonID)
{
    switch (buttonID)
    {
        case OIS::MB_Left:    return CEGUI::LeftButton;
        case OIS::MB_Right:   return CEGUI::RightButton;
        case OIS::MB_Middle:  return CEGUI::MiddleButton;
        default:              return CEGUI::LeftButton;
    }
}
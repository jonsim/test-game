#ifndef INPUT_H
#define INPUT_H


//---------- INCLUDES ----------//

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>

#include <SdkTrays.h>
#include <SdkCameraMan.h>

#include <CEGUI.h>



//---------- DEFINITIONS ----------//
class Input : public OIS::KeyListener, public OIS::MouseListener
{
public:
    Input (OIS::ParamList paramList);
    virtual ~Input (void);
    void capture (void);

    //OIS Input devices
    OIS::InputManager* mInputManager;
    OIS::Mouse*        mMouse;
    OIS::Keyboard*     mKeyboard;


protected:
    // Input
    // OIS::KeyListener
    virtual bool keyPressed  (const OIS::KeyEvent &arg);
    virtual bool keyReleased (const OIS::KeyEvent &arg);

    // OIS::MouseListener
    virtual bool mouseMoved    (const OIS::MouseEvent &arg );
    virtual bool mousePressed  (const OIS::MouseEvent &arg, OIS::MouseButtonID id);
    virtual bool mouseReleased (const OIS::MouseEvent &arg, OIS::MouseButtonID id);
    
    // CEGUI
	CEGUI::MouseButton convertButton (OIS::MouseButtonID buttonID);
};

#endif // #ifndef INPUT_H

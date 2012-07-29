//---------- INCLUDES ----------//
#include "Player.h"



//---------- FUNCTIONS ----------//

/// @brief  Constructor
Player::Player (Ogre::SceneManager* sceneManager) : mPlayerAimHeight(100)
{
    setupNodes(sceneManager);
    setupTargettingSystem(sceneManager);
}



/// @brief  Deconstructor
Player::~Player (void)
{
}


void Player::frameUpdate (const float tslf)
{
    // Get the old (current) position.
    Ogre::Vector3 oldPosition = mPlayerNode->getPosition();

    // Calculate the components from the input.
    float forward = 0, right = 0, clockwise = 0;
    if (Core::mGraphicsCore->mUserInput->mKeyboard->isKeyDown(OIS::KC_W))
        forward -= 200 * tslf;
    if (Core::mGraphicsCore->mUserInput->mKeyboard->isKeyDown(OIS::KC_S))
        forward += 200 * tslf;
    if (Core::mGraphicsCore->mUserInput->mKeyboard->isKeyDown(OIS::KC_Q))
        right += 200 * tslf;
    if (Core::mGraphicsCore->mUserInput->mKeyboard->isKeyDown(OIS::KC_E))
        right -= 200 * tslf;
    if (Core::mGraphicsCore->mUserInput->mKeyboard->isKeyDown(OIS::KC_A))
        clockwise += 100 * tslf;
    if (Core::mGraphicsCore->mUserInput->mKeyboard->isKeyDown(OIS::KC_D))
        clockwise -= 100 * tslf;

    // rotate the player (so that we can calculate the new position with the new rotation applied).
    mPlayerNode->rotate(Ogre::Vector3::UNIT_Y, Ogre::Radian(Ogre::Degree(clockwise)));

    // Calculate the components of the translation.
    Ogre::Radian rot = mPlayerNode->getOrientation().getYaw(true);
    float zComponent = Ogre::Math::Cos(rot);
    float xComponent = Ogre::Math::Sin(rot);
    
    // Calculate the new position.
    Ogre::Vector3 newPosition(oldPosition.x + (xComponent * forward) + (xComponent * right),
                              0,
                              oldPosition.z + (zComponent * forward) + (zComponent * right));
    newPosition.y = Core::mWorld->getTerrainHeight(newPosition) + mPlayerHeight;

    // Apply this new position to the player.
    mPlayerNode->setPosition(newPosition);
}



void Player::setupNodes (Ogre::SceneManager* sceneManager)
{
    // Create the nodes
    mPlayerNode         = sceneManager->getRootSceneNode()->createChildSceneNode("PlayerNode");
    mPlayerCameraNode   = mPlayerNode->createChildSceneNode("PlayerCameraNode");
    mCharacterNode      = mPlayerNode->createChildSceneNode("PlayerCharacterNode");
    mTargettingPathNode = mPlayerNode->createChildSceneNode("PlayerPathNode");
    mTargettingEndNode  = sceneManager->getRootSceneNode()->createChildSceneNode("TargettingEndNode");
    mTargettingPathNode->setVisible(false);
    mPlayerCameraNode->setPosition(0, 100, 200);


    // Add the entities
    // Add player
    Ogre::Entity* playerEntity = sceneManager->createEntity("PlayerCharacter", "potato.mesh");
    Ogre::AxisAlignedBox bb = playerEntity->getBoundingBox();
    mPlayerHeight = (bb.getMaximum().y - bb.getMinimum().y) * 0.1f * 0.5f;
    mCharacterNode->attachObject(playerEntity);
    mCharacterNode->setScale(0.1f, 0.1f, 0.1f);
    mCharacterNode->setPosition(0, 0, 0);

    // Add target marker
    mTargettingEndNode = sceneManager->getRootSceneNode()->createChildSceneNode("TargetNode");
    Ogre::Entity* targetEntity = sceneManager->createEntity("TargetEntity", "sphere.mesh");
    mTargettingEndNode->attachObject(targetEntity);
    targetEntity = sceneManager->createEntity("TargetEntity2", "column.mesh");
    mTargettingEndNode->attachObject(targetEntity);
    mTargettingEndNode->setScale(0.1f, 0.1f, 0.1f);
    mTargettingEndNode->setPosition(0, 0, 0);


    // Billboard set
    /*mTargettingEndBBS = sceneManager->createBillboardSet("TargetBillboardSet", 2);
    sceneManager->getRootSceneNode()->attachObject(mTargettingEndBBS);
    mTargettingEndBBS->setMaterialName("Examples/Flare");
    mTargettingEndBBS->setVisible(true);
    mTargettingEndBBS->createBillboard(0, 300, 0, Ogre::ColourValue(0.5f, 0.6f, 1.0f));
    mTargettingEndBBS->setDefaultDimensions(100, 100);
    mTargettingEndBBS->setBillboardType(Ogre::BBT_ORIENTED_COMMON);
    mTargettingEndBBS->setCommonUpVector(Ogre::Vector3(0, 1, 0));*/
}



void Player::setupTargettingSystem (Ogre::SceneManager* sceneManager)
{
    // Add billboard chain
    mTargettingPathChain = sceneManager->createBillboardChain("arc");
    mTargettingPathChain->setNumberOfChains(1);
    mTargettingPathChain->setMaxChainElements(ARC_RESOLUTION);
    mTargettingPathChain->setTextureCoordDirection(Ogre::BillboardChain::TCD_V);
    mTargettingPathChain->setMaterialName("Examples/LightRibbonTrail");
    mTargettingPathNode->attachObject(mTargettingPathChain);
    mTargettingPathChain->setVisible(true);
}


void Player::attachCamera (Ogre::Camera* camera)
{
    mPlayerCameraNode->attachObject(camera);
}


void Player::placeTargettingEnd (const Ogre::Vector3& endPosition)
{
    // Get the terrain normal
    Ogre::Vector3 terrainNormal = Core::mWorld->getTerrainNormal(endPosition);

    // Place and angle the aim nodule
    mTargettingEndNode->setPosition(endPosition);
    mTargettingEndNode->setDirection(terrainNormal, Ogre::Node::TS_WORLD, Ogre::Vector3::UNIT_Y);
    //mTargetBillboardSet->setCommonUpVector(Ogre::Vector3(0, 1, 0));
    //mTargetBillboardSet->setCommonUpVector(n);
    //mTargetBillboardSet->getBillboard(0)->setPosition(terrainHit.position + Ogre::Vector3(0, 5, 0));
    mTargettingEndNode->setVisible(true);
}



void Player::showTargettingSystem (OIS::MouseState ms)
{
    Ogre::Vector3 terrainHitPosition;

    // Place the target node ting
    // Ray cast from the current mouse point through the camera
    Ogre::Ray mouseRay = Core::mGraphicsCore->mCamera->getCameraToViewportRay(ms.X.abs / ((float) ms.width), ms.Y.abs / ((float) ms.height));
    bool terrainHit = Core::mWorld->terrainRaycast(mouseRay, &terrainHitPosition);

    // Check if the user clicked on terrain
    if (terrainHit)
    {
        // Draw the path.
        drawTargettingPath(terrainHitPosition, mPlayerAimHeight);
        
        // Place the end of the targetting path.
        placeTargettingEnd(terrainHitPosition);

        // Hide the normal cursor.
        CEGUI::MouseCursor::getSingleton().hide();
        
        // Display the aiming cursor.
        mTargettingPathNode->setVisible(true);
        mTargettingEndNode->setVisible(true);
    }
}


void Player::hideTargettingSystem (void)
{
    mTargettingPathNode->setVisible(false);
    mTargettingEndNode->setVisible(false);
}



void Player::drawTargettingPath (const Ogre::Vector3& endPosition, const float height)
{
    char bob[128];
    sprintf(bob, "height = %.2f\n", height);
    OutputDebugString(bob);

    // The curve's equation is of the form  y = a - ((x - b) / c)^2  (an upside down parabola).
    // a gives the curve's maximum height (apex), b gives the curves centre on the x-axis and
    // c can be shown to be equal to b / sqrt(a). Imposing the arbitrary constraint that
    // a*b = 500 the curve's area becomes fixed, giving the illusion of different trajectories.
    float x, y, z;
    Ogre::Vector3 localPosition, worldPosition;
    
    Ogre::Vector3 startPosition = mPlayerNode->getPosition();
    //Ogre::Vector3 targetPos = mTargettingEndNode->getPosition();
    Ogre::Vector3 endPosition_pc = mPlayerNode->convertWorldToLocalPosition(endPosition);
    endPosition_pc.y = 0;
    float distance = endPosition_pc.length();

    // Deriving a curve's parameters requires 3 known points. We only have 2 available (the start and end),
    // however we also know the maximum height we want the curve to reach. Using this and making some assumptions
    // about the shape of the curve that we want we can infer a third point, the 'apex' of the curve. This will
    // not actually be the curve's true apex, but the curve will pass through it and the closeness of the model
    // used to approximate the third point to the true curve will affect how close this apex is.
    //
    // Start with three points, p1=(x1, y1), p2=(x2, y2) p3=(x3, y3). p1 and p3 are the start and end co-ordinates
    // in 2D co-ordinates relative to the player node (so p1 is always (0, 0))(they could be in world co-ordinates
    // but it adds unnecessary complexity to the calculations later).
    // The naive approach to getting a third point would be to assume that the apex of the curve would be central
    // along the x-axis (i.e. distance / 2), true when there is no difference in height between the player and the
    // target, however when a significant drop in height occurs between the two it means that the curve will be
    // significantly too high (as the true apex would fall closer to the player along the curve, but the curve 
    // must still pass through the calculated apex midway along the curve, causing the true apex to be much 
    // higher). The solution is to model the curve (with no knowledge of it) and guess where the true apex will
    // occur, thus minimising the difference between calculated and true apex and the over/under shooting that 
    // occurs.
    // To do this we use the simple, arbitrarily selected curve y = -0.2 x^2, where x = -sqrt( abs(5y) ). Using
    // this we can calculate where the apex of the curve falls on the x-axis in relation to the starting height
    // of the curve. Selecting three points from this curve (in this case (0, 0.5), (0.5, 0.415), (0.875, 0.261)
    // using the x-axis to represent the height the player node starts at relative to the TOTAL vertical distance
    // (i.e. not in player co-ordinates) and the y-axis to represent the horizontal position of the apex relative
    // to the total horizontal distance travelled) allow us to build a further curve showing the rate of change
    // of the apex position against the changing starting height. This curve has the parameters (a=-0.275,
    // b=-0.032, c=0.5) and, given x = (drop distance) / (drop distance + apex height to player) produces the
    // horizontal position of the apex along the curve (between 0 and 1). This apex is for the previously noted
    // curve y = -0.2 x^2, however since our desired curve is of polynomial order and the sam rough shape, it is
    // not a wild assumption that the apex of OUR curve will be in a similar location.
    // The output we just generated can be multiplied by the total distance travelled by our curve to give an
    // approximation of the x co-ordinate of the apex. This completes the third point to derive our curves
    // parameters, allowing it to be evaluated and plotted.
    // If greater accuracy is required, the curve generated at this point can be fed back into the algorithm in
    // an iterative manner in place of the function y=-0.2x^2 as a much more accurate method of approximating the
    // apex's location. This would be relatively straight forward though computationally expensive as this 
    // calculation would have to be done online and, since the curve would be more complicated, differentiation
    // would have to be used to calculate the apex rather than inverting the function. The advantage of this is
    // that the output would be far more accurate and would converge rapidly - I would estimate that no more than
    // one such iteration would be required.

    float x1 = 0;                   float y1 = 0;
    float x3 = distance;            float y3 = endPosition.y - startPosition.y;
    float x2 = distance * 0.5f;     float y2 = (height + MAX(y1, y3));

    float heightDropRatio, apexDistanceRatio;
    if (y3 > 0)
        heightDropRatio   = ( y3) / ( y3 + height);
    else
        heightDropRatio   = (-y3) / (-y3 + height);
    apexDistanceRatio = (-0.275 * heightDropRatio * heightDropRatio) + (-0.032 * heightDropRatio) + 0.5;
    if (y3 > 0)
        apexDistanceRatio = 1 - apexDistanceRatio;
    x2 = apexDistanceRatio * distance;

    //char bob[128];
    sprintf(bob, "heightDropRatio=%.2f, apexDistanceRatio=%.2f\n", heightDropRatio, apexDistanceRatio);
    OutputDebugString(bob);

                                    //float y2 = height - (y3 / 2.0f);
    /*Ogre::Matrix3 xMat( x1*x1, x1, 1,
                        x2*x2, x2, 1,
                        x3*x3, x3, 1  );
    Ogre::Vector3 yMat( y1,
                        y2,
                        y3  );
    Ogre::Matrix3 xMatI = xMat.Inverse();
    Ogre::Vector3 rMat = xMatI * yMat;*/
    float denom = (x1 - x2) * (x1 - x3) * (x2 - x3);
    float A     = (x3 * (y2 - y1) + x2 * (y1 - y3) + x1 * (y3 - y2)) / denom;
    float B     = (x3*x3 * (y1 - y2) + x2*x2 * (y3 - y1) + x1*x1 * (y2 - y3)) / denom;
    float C     = (x2 * x3 * (x2 - x3) * y1 + x3 * x1 * (x3 - x1) * y2 + x1 * x2 * (x1 - x2) * y3) / denom;
    
    // Clear the chains in preparation to redraw.
    mTargettingPathChain->clearAllChains();
    mTargettingPathChain->addChainElement(0, Ogre::BillboardChain::Element(Ogre::Vector3::ZERO, 1, 0, Ogre::ColourValue(1.0f, 1.0f, 1.0f)));
    float eval_step = distance / ((float) ARC_RESOLUTION);
    for (float i = eval_step; i < distance; i += eval_step)
    {
        // Evaluate the curve at the current point.
        x = 0;
        y = A * i * i + B * i + C;
        z = -i;

        // Check the curve to see if it has clipped through the terrain
        localPosition = Ogre::Vector3(x, y, z);

        // Redraw the curve with the new values.
        mTargettingPathChain->addChainElement(0, Ogre::BillboardChain::Element(localPosition, 1, 0, Ogre::ColourValue(1.0f, 1.0f, 1.0f)));
    }

    

    // rotate the shit
    // Get the direction of the target relative to the player, projecting it down into 2D to get the planar
    // distance (XZ plane) of the target (elevation is accounted for when solving the equations to produce 
    // the curve by further projection to the XY plane and subsequent rotation). Failing to project to XZ
    // here will cause erroneous distances to be calculated later on.
    //Ogre::Vector3 endPosition_pc = mPlayerNode->convertWorldToLocalPosition(endPosition);
    //endPosition_pc.y = 0;
    Ogre::Radian targetAngle = endPosition_pc.angleBetween(Ogre::Vector3::NEGATIVE_UNIT_Z);
    if (endPosition_pc.x > 0)
        targetAngle *= -1;

    // Angle the path node appropriately.
    mTargettingPathNode->setOrientation(Ogre::Quaternion(targetAngle, Ogre::Vector3::UNIT_Y));
}
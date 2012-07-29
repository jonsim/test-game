#ifndef CORE_H
#define CORE_H


//---------- INCLUDES ----------//

// Externals
class GraphicsCore;
class Player;
class World;


//---------- DEFINITIONS ----------//
class Core
{
public:
    static GraphicsCore* mGraphicsCore;
    static Player*       mPlayer;
    static World*        mWorld;

    static void setup (GraphicsCore* graphicsCore)  { mGraphicsCore = graphicsCore; }
};

#endif // #ifndef CORE_H

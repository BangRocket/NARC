#include "../include/g_main.h"
#include "../../engine/include/i_system.h"
#include "../../engine/include/i_video.h"
#include "../../engine/include/i_input.h"
#include "../../engine/include/i_audio.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    I_Init();
    
    game_config_t gameConfig = {
        .screenWidth = 1280,
        .screenHeight = 720,
        .fullscreen = false,
        .vsync = true,
        .targetFPS = 60,
        .windowTitle = "NARC - Not Another RayCaster",
        .dataPath = "./DATA/"
    };
    
    video_config_t videoConfig = {
        .width = gameConfig.screenWidth,
        .height = gameConfig.screenHeight,
        .fullscreen = gameConfig.fullscreen,
        .vsync = gameConfig.vsync,
        .targetFPS = gameConfig.targetFPS,
        .title = gameConfig.windowTitle
    };
    
    if (!I_InitVideo(&videoConfig)) {
        I_Error("Failed to initialize video");
        return 1;
    }
    
    I_InitInput();
    
    audio_config_t audioConfig = {
        .sampleRate = 44100,
        .channels = 2,
        .spatialAudio = true
    };
    
    if (!I_InitAudio(&audioConfig)) {
        I_Warning("Failed to initialize audio - continuing without sound");
    }
    
    if (!G_Init(&gameConfig)) {
        I_Error("Failed to initialize game");
        return 1;
    }
    
    game_context_t* gameContext = G_CreateGameContext();
    if (!gameContext) {
        I_Error("Failed to create game context");
        return 1;
    }
    
    if (!G_InitializeGameContext(gameContext, &gameConfig)) {
        I_Error("Failed to initialize game context");
        return 1;
    }
    
    if (!G_LoadGameLevel(gameContext, 1)) {
        I_Error("Failed to load first level");
        return 1;
    }
    
    I_Log("Starting main game loop");
    
    while (!I_ShouldClose()) {
        I_UpdateInput();
        
        G_ProcessInput();
        G_RunFrame();
        
        float deltaTime = I_GetDeltaTime();
        G_UpdateGameContext(gameContext, deltaTime);
        
        I_BeginDrawing();
        G_Draw();
        G_RenderGameContext(gameContext);
        I_EndDrawing();
    }
    
    I_Log("Shutting down");
    
    G_DestroyGameContext(gameContext);
    G_Shutdown();
    I_ShutdownAudio();
    I_ShutdownInput();
    I_ShutdownVideo();
    I_Shutdown();
    
    return 0;
}
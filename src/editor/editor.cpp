
#include "editor.h"
#include "../core/synth_definitions.h"

DangoFM::Editor::Editor()
{

}

int DangoFM::Editor::Init()
{
// Setup SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }
#ifdef SDL_HINT_IME_SHOW_UI
    SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");
#endif

    // Setup window
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    window = SDL_CreateWindow("Dear ImGui SDL2+OpenGL example", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    if (window == nullptr)
    {
        printf("Error: SDL_CreateWindow(): %s\n", SDL_GetError());
        return -1;
    }


    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL2_Init();

    synthesizer.Init();
    driver.Init(192, 129, &synthesizer);
    InitAudio();

    // Init windows
    mainW.Init(audio_device);
    synthW.Init();
    channelsW.Init();

    mainW.SetAudioPaused(false);

    return 0;
}



void audio_callback(void *userData, uint8 *buffer, int bufferLengthInBytes) {
  DangoFM::Driver *driver = (DangoFM::Driver*)userData;

  SDL_memset(buffer, 0, bufferLengthInBytes);
  int16* sampleBuffer = (int16*)buffer;
  driver->Play(DANGO_SAMPLES_PER_CALLBACK, 1.0f, sampleBuffer);
}

void DangoFM::Editor::InitAudio()
{
    /* On Windows the DirectSound driver uses 16 bit output.
     Always choose that
  */
#ifdef DANGO_WINDOWS
  if (SDL_AudioInit("directsound")){
    printf("SDL failed to init directsound\n");
  }
  else {
    printf("SDL initialized directsound\n");
  }
#endif

  SDL_AudioSpec requirements;
  SDL_AudioSpec obtained;

  SDL_memset(&requirements, 0, sizeof(requirements));
  requirements.freq = DANGO_SAMPLES_PER_SECOND;
  /* Request Signed 16 bit, least significant byte
   */
  requirements.format = AUDIO_S16LSB;
  requirements.channels = DANGO_OUT_CHANNELS;
  requirements.samples = DANGO_SAMPLES_PER_CALLBACK;
  requirements.callback = audio_callback;
  requirements.userdata = (void*)&driver;

  int change_flags = SDL_AUDIO_ALLOW_FORMAT_CHANGE;
  audio_device = SDL_OpenAudioDevice(NULL, SDL_FALSE, &requirements, &obtained, change_flags);
  if (audio_device == 0) {
    printf("Failed to open audio: %s\n", SDL_GetError());
  }
  printf("Audio specifications:\n");
  printf("\tFrequency: %d", obtained.freq);
  if (obtained.channels == 1) {
    printf("\tMono output\n");
  }
  if (obtained.channels == 2) {
    printf("\tStereo output\n");
  }
  int audio_format = 0;
  if (obtained.format == AUDIO_S16LSB) {
    audio_format = 16;
  }
  else if (obtained.format == AUDIO_F32LSB) {
    audio_format = 32;
  }
  printf("\tOutput format is %dbit\n", audio_format);
  printf("\tSamples per callback is %d", obtained.samples);
  if (obtained.samples == DANGO_SAMPLES_PER_CALLBACK) {
    printf(" as requested.\n");
  }
  else {
    printf(" NOT as requested.\n");
  }
}


void DangoFM::Editor::Run()
{
// Main loop
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }
        if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
        {
            SDL_Delay(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        mainW.Draw(driver);
        if (synthW.isOpen)
        {
          synthW.Draw(synthesizer, driver);
        }
        keyW.Draw(synthesizer);
        channelsW.Draw(synthesizer, driver);
        instrumentW.Draw(synthesizer, synthesizer.GetChannel(selectedChannel)->instrument);

        //ImGui::ShowDemoWindow();

         // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
	}
	Quit();
}

void DangoFM::Editor::Quit()
{
	// Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_PauseAudioDevice(audio_device, true);
    SDL_CloseAudio();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}




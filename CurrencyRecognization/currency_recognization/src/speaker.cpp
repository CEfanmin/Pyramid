/*
 #include "speaker.h"

 espeak_POSITION_TYPE position_type;

 char *path = NULL;
 int Buflength = 500, Options = 0;
 void* user_data;
 t_espeak_callback *SynthCallback;
 espeak_PARAMETER Parm;

 char Voice[] = { "default" };

 //char text[20] = {"Hello World!"};

 unsigned int Size_t, position = 0, end_position = 0, flags = espeakCHARS_AUTO,
 *unique_identifier;

 void initSpeaker() {
 espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, Buflength, path, Options);
 espeak_SetVoiceByName(Voice);
 }

 int speak_synchronously(string text) {
 int retval = speak(text);
 espeak_Synchronize();

 return retval;
 }

 int speak(string text) {
 if (espeak_IsPlaying() == 1) {
 return -1;
 }

 //int I, Run = 1, L;
 Size_t = text.length() + 1;
 cout << "Saying  '%s'" << text << endl;

 char *ctext;
 ctext = &text[0];
 espeak_Synth(ctext, Size_t, position, position_type, end_position, flags,
 unique_identifier, user_data);
 //espeak_Synchronize( );

 return 0;
 }

 int AutoSpeak(string text) {
 //
 if (espeak_IsPlaying() == 1) {
 //sleep(10);
 return -1;
 }
 int Size_p = text.length() + 1;
 const char *ctext = text.c_str();
 espeak_Initialize(AUDIO_OUTPUT_PLAYBACK, 100, NULL, 0);    // initialize
 espeak_SetVoiceByName("zh+m0");    // set the voice as chinese female
 espeak_Synth(ctext, Size_p, 0, POS_CHARACTER, 0, espeakCHARS_UTF8,
 NULL, NULL);        // speak out
 sleep(3);     // wait for the speak before call back
 espeak_Terminate();  // close
 return 0;
 }

 */

// Russian TTS library java interface

#include <cstdlib>
#include <cstring>

#include <jni.h>

#include "ru_tts.h"


// Local classes

// Callback interface
class cb_handler_t
{
public:
  cb_handler_t(JNIEnv* cb_env, jobject cb_obj, jclass cls):
    env(cb_env),
    obj(cb_obj)
  {
    method = env->GetMethodID(cls, "speechCallback", "([B)Z");
  }

  JNIEnv* env;
  jobject obj;
  jmethodID method;
};

// Simplified auto_ptr functionality
class auto_buffer_t
{
public:
  auto_buffer_t(size_t size):
    length(size)
  {
    if (size)
      content = new char[size];
    else content = 0;
  }

  ~auto_buffer_t(void)
  {
    if (content)
      delete content;
  }

  size_t length;
  char* content;
};


// Local data

// Recoding table for koi8-r
static const char koi[] =
  {
    0xE1, 0xE2, 0xF7, 0xE7, 0xE4, 0xE5, 0xF6, 0xFA,
    0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, 0xF0,
    0xF2, 0xF3, 0xF4, 0xF5, 0xE6, 0xE8, 0xE3, 0xFE,
    0xFB, 0xFD, 0xFF, 0xF9, 0xF8, 0xFC, 0xE0, 0xF1,
    0xC1, 0xC2, 0xD7, 0xC7, 0xC4, 0xC5, 0xD6, 0xDA,
    0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, 0xD0,
    0xD2, 0xD3, 0xD4, 0xD5, 0xC6, 0xC8, 0xC3, 0xDE,
    0xDB, 0xDD, 0xDF, 0xD9, 0xD8, 0xDC, 0xC0, 0xD1
  };

// Additional character codes
static const char blank = 0x20;
static const char koi_uc_yo = 0xB3;
static const char koi_lc_yo = 0xA3;
static const jchar utf_uc_yo = 0x0401;
static const jchar utf_lc_yo = 0x0451;

// Code bases for Russian letters
static const int utf_codebase = 0x0410;
static const int extended_codebase = 0x80;


// Local functions

// Transfer control to the Java callback
int
callback(void* buffer, size_t size, void* user_data)
{
  int rc = 0;
  if (buffer && size && user_data)
    {
      cb_handler_t* cb_handler = reinterpret_cast<cb_handler_t*>(user_data);
      jbyteArray cb_data = cb_handler->env->NewByteArray(size);
      cb_handler->env->SetByteArrayRegion(cb_data, 0, size, reinterpret_cast<jbyte*>(buffer));
      rc = static_cast<int>(cb_handler->env->CallBooleanMethod(cb_handler->obj, cb_handler->method, cb_data));
    }
  return rc;
}


// Exported native method

extern "C" {

  JNIEXPORT void
  JNICALL Java_tts_synth_RussianVoiceEngine_speak(JNIEnv* env, jobject obj, jstring text)
  {
    jclass cls = env->GetObjectClass(obj);
    cb_handler_t cb_handler(env, obj, cls);
    int textsize = static_cast<int>(env->GetStringLength(text));
    auto_buffer_t textbuffer(textsize + 1);
    auto_buffer_t audiobuffer(static_cast<size_t>(env->GetIntField(obj, env->GetFieldID(cls, "audioBufferSize", "I"))));

    const jchar* utftext = env->GetStringChars(text, 0);
    for (int i = 0; i < textsize; i++)
      switch (utftext[i])
        {
        case utf_uc_yo:
          textbuffer.content[i] = koi_uc_yo;
          break;
        case utf_lc_yo:
          textbuffer.content[i] = koi_lc_yo;
          break;
        default:
          if ((utftext[i] >= utf_codebase) && (utftext[i] < (utf_codebase + sizeof(koi))))
            textbuffer.content[i] = koi[utftext[i] - utf_codebase];
          else if ((utftext[i] < extended_codebase) && (utftext[i] > blank))
            textbuffer.content[i] = utftext[i];
          else textbuffer.content[i] = blank;
        }
    env->ReleaseStringChars(text, utftext);

    while (textsize)
      if (static_cast<unsigned char>(textbuffer.content[textsize - 1]) <= blank)
        textsize--;
      else break;

    if (textsize > 0)
      {
        textbuffer.content[textsize] = 0;
        ru_tts_transfer(textbuffer.content, audiobuffer.content, audiobuffer.length, callback, &cb_handler,
                        static_cast<int>(env->GetBooleanField(obj, env->GetFieldID(cls, "altVoice", "Z"))),
                        static_cast<int>(env->GetIntField(obj, env->GetFieldID(cls, "speechRate", "I"))),
                        static_cast<int>(env->GetIntField(obj, env->GetFieldID(cls, "pitch", "I"))),
                        static_cast<int>(env->GetBooleanField(obj, env->GetFieldID(cls, "applyIntonation", "Z"))));
      }
  }

}

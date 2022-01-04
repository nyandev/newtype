#include "pch.h"
#include "newtype.h"
#include "newtype_manager.h"

using namespace newtype;

static unique_ptr<ManagerImpl> g_instance;

extern "C" {

  NEWTYPE_EXPORT Manager* NEWTYPE_CALL newtypeInitialize( uint32_t version, Host* host )
  {
    if ( version != c_headerVersion )
      return nullptr;
    g_instance = std::make_unique<ManagerImpl>( host );
    g_instance->initialize();
    return g_instance.get();
  }

  NEWTYPE_EXPORT void NEWTYPE_CALL newtypeShutdown( Manager* manager )
  {
    if ( g_instance )
      g_instance->shutdown();
    g_instance.reset();
  }

}

BOOL APIENTRY DllMain( HMODULE module, DWORD reason, LPVOID reserved )
{
  switch ( reason )
  {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
      break;
    case DLL_PROCESS_DETACH:
      g_instance.reset();
      break;
  }
  return TRUE;
}
#include "ControlServer.h"
#include "SubscriptionManager.h"
#include "plex.h"
#include "plexOsd.h"

//////////////////////////////////////////////////////////////////////////////
//	cPlugin
//////////////////////////////////////////////////////////////////////////////

volatile bool cMyPlugin::CalledFromCode = false;

/**
**	Initialize any member variables here.
**
**	@note DON'T DO ANYTHING ELSE THAT MAY HAVE SIDE EFFECTS, REQUIRE GLOBAL
**	VDR OBJECTS TO EXIST OR PRODUCE ANY OUTPUT!
*/
cMyPlugin::cMyPlugin(void)
{
}

/**
**	Clean up after yourself!
*/
cMyPlugin::~cMyPlugin(void)
{
	plexclient::plexgdm::GetInstance().stopRegistration();
	plexclient::ControlServer::GetInstance().Stop();
}

/**
**	Return plugin version number.
**
**	@returns version number as constant string.
*/
const char *cMyPlugin::Version(void)
{
	return VERSION;
}

/**
**	Return plugin short description.
**
**	@returns short description as constant string.
*/
const char *cMyPlugin::Description(void)
{
	return DESCRIPTION;
}


bool cMyPlugin::Start(void)
{
	return true;
}

/**
**	Start any background activities the plugin shall perform.
*/
bool cMyPlugin::Initialize(void)
{
	// First Startup? Save UUID
	SetupStore("UUID", Config::GetInstance().GetUUID().c_str());

	plexclient::plexgdm::GetInstance().clientDetails(Config::GetInstance().GetUUID(), Config::GetInstance().GetHostname(), "3200", DESCRIPTION, VERSION);
	plexclient::plexgdm::GetInstance().Start();
	plexclient::ControlServer::GetInstance().Start();

	return true;
}

/**
**	Create main menu entry.
*/
const char *cMyPlugin::MainMenuEntry(void)
{
	return Config::GetInstance().HideMainMenuEntry ? NULL : MAINMENUENTRY;
}

/**
**	Perform the action when selected from the main VDR menu.
*/
cOsdObject *cMyPlugin::MainMenuAction(void)
{
	//dsyslog("[plex]%s:\n", __FUNCTION__);
	return cPlexMenu::ProcessMenu();
}

/**
**	Called for every plugin once during every cycle of VDR's main program
**	loop.
*/
void cMyPlugin::MainThreadHook(void)
{
	// dsyslog("[plex]%s:\n", __FUNCTION__);
	// Start Tasks, e.g. Play Video
	if(plexclient::ActionManager::GetInstance().IsAction()) {
		PlayFile(plexclient::ActionManager::GetInstance().GetAction());
	}
}

/**
**	Return our setup menu.
*/
cMenuSetupPage *cMyPlugin::SetupMenu(void)
{
	//dsyslog("[plex]%s:\n", __FUNCTION__);

	return new cMyMenuSetupPage;
}

/**
**	Parse setup parameters
**
**	@param name	paramter name (case sensetive)
**	@param value	value as string
**
**	@returns true if the parameter is supported.
*/
bool cMyPlugin::SetupParse(const char *name, const char *value)
{
	//dsyslog("[plex]%s: '%s' = '%s'\n", __FUNCTION__, name, value);

	if (strcasecmp(name, "HideMainMenuEntry") == 0) 	Config::GetInstance().HideMainMenuEntry = atoi(value) ? true : false;
	else if (strcasecmp(name, "UsePlexAccount") == 0) 	Config::GetInstance().UsePlexAccount = atoi(value) ? true : false;
	else if (strcasecmp(name, "UseCustomTranscodeProfile") == 0) 	Config::GetInstance().UseCustomTranscodeProfile = atoi(value) ? true : false;
	else if (strcasecmp(name, "Username") == 0) 		Config::GetInstance().s_username = std::string(value);
	else if (strcasecmp(name, "Password") == 0) 		Config::GetInstance().s_password = std::string(value);
	else if (strcasecmp(name, "UUID") == 0) 			Config::GetInstance().SetUUID(value);
	else return false;

	return true;
}

/**
**	Play a file.
**
**	@param filename	path and file name
*/
void cMyPlugin::PlayFile(plexclient::Video Vid)
{
	isyslog("[plex]: play file '%s'\n", Vid.m_sKey.c_str());
	if(Vid.m_iMyPlayOffset == 0 && Vid.m_lViewoffset > 0 ) {
		cString message = cString::sprintf(tr("To start from %ld minutes, press Ok."), Vid.m_lViewoffset / 60000);
		eKeys response = Skins.Message(eMessageType::mtInfo, message, 5);
		if(response == kOk) {
			Vid.m_iMyPlayOffset = Vid.m_lViewoffset/1000;
		}
	}
	cControl* control = cHlsPlayerControl::Create(Vid);
	if(control) {
		cControl::Launch(control);
	}
}

VDRPLUGINCREATOR(cMyPlugin);		// Don't touch this!

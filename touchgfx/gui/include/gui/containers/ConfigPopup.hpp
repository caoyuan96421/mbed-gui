#ifndef CONFIGPOPUP_HPP
#define CONFIGPOPUP_HPP

#include <gui_generated/containers/ConfigPopupBase.hpp>
#include "TelescopeBackend.h"

class ConfigPopup: public ConfigPopupBase
{
public:
	ConfigPopup();
	virtual ~ConfigPopup()
	{
	}

	void editConfig(ConfigItem *config);

protected:

	ConfigItem *config;
	char editbuf[20];

	touchgfx::Callback<ConfigPopup, const AbstractButton&> callbackok;
	touchgfx::Callback<ConfigPopup, const AbstractButton&> callbackcancel;
	touchgfx::Callback<ConfigPopup, const AbstractButton&> callbackedit;

	void ok(const AbstractButton &);
	void cancel(const AbstractButton &);
	void edit(const AbstractButton &);
};

#endif // CONFIGPOPUP_HPP

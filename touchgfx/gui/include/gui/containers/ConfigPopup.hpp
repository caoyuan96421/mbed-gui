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
	void setCallback(touchgfx::GenericCallback<ConfigItem *, bool> *cb)
	{
		callback = cb;
	}

	Rect getSolidRect() const
	{
		return Rect(0, 0, getWidth(), getHeight());
	}

	void handleClickEvent(const ClickEvent &)
	{
	}

protected:

	ConfigItem *config;
	char editbuf[20];

	touchgfx::Callback<ConfigPopup, const AbstractButton&> callbackok;
	touchgfx::Callback<ConfigPopup, const AbstractButton&> callbackcancel;
	touchgfx::Callback<ConfigPopup, const AbstractButton&> callbackedit;
	touchgfx::GenericCallback<ConfigItem *, bool> *callback;

	virtual void ok(const AbstractButton &);
	virtual void cancel(const AbstractButton &);
	virtual void edit(const AbstractButton &);
};

#endif // CONFIGPOPUP_HPP

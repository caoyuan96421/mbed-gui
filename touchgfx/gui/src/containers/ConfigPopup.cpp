#include <gui/containers/ConfigPopup.hpp>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cmath>

ConfigPopup::ConfigPopup() :
		config(NULL), callbackok(this, &ConfigPopup::ok), callbackcancel(this, &ConfigPopup::cancel), callbackedit(this, &ConfigPopup::edit), callback(NULL)
{
	okButton.setAction(callbackok);
	cancelButton.setAction(callbackcancel);
	button0.setAction(callbackedit);
	button1.setAction(callbackedit);
	button2.setAction(callbackedit);
	button3.setAction(callbackedit);
	button4.setAction(callbackedit);
	button5.setAction(callbackedit);
	button6.setAction(callbackedit);
	button7.setAction(callbackedit);
	button8.setAction(callbackedit);
	button9.setAction(callbackedit);
	buttondot.setAction(callbackedit);
	buttoncr.setAction(callbackedit);
	upbutton.setAction(callbackedit);
	downbutton.setAction(callbackedit);
	buttonminus.setAction(callbackedit);

	helpArea.setWideTextAction(WIDE_TEXT_WORDWRAP);
	setVisible(false);
	setTouchable(true); // Capture all click event

//	box3.forceReportAsSolid(true);

}

void ConfigPopup::editConfig(ConfigItem* config)
{
	if (config)
	{
		this->config = config;
		this->setVisible(true);

		Unicode::strncpy(nameAreaBuffer, config->name, NAMEAREA_SIZE);
		Unicode::strncpy(helpAreaBuffer, config->help, HELPAREA_SIZE);

		button0.setVisible(true);
		button1.setVisible(true);
		button2.setVisible(true);
		button3.setVisible(true);
		button4.setVisible(true);
		button5.setVisible(true);
		button6.setVisible(true);
		button7.setVisible(true);
		button8.setVisible(true);
		button9.setVisible(true);
		buttondot.setVisible(true);
		buttoncr.setVisible(true);
		upbutton.setVisible(true);
		downbutton.setVisible(true);
		buttonminus.setVisible(true);

		char buf[20];

		switch (config->type)
		{
		case DATATYPE_DOUBLE:
			snprintf(buf, VALUEAREA_SIZE, "%f", config->value.ddata);
			if (config->min.ddata >= 0)
			{
				buttonminus.setVisible(false);
			}
			break;
		case DATATYPE_INT:
			snprintf(buf, VALUEAREA_SIZE, "%d", config->value.idata);
			buttondot.setVisible(false);
			if (config->min.idata >= 0)
			{
				buttonminus.setVisible(false);
			}
			break;
		case DATATYPE_STRING:
			snprintf(buf, VALUEAREA_SIZE, "%s", config->value.strdata);
			break;
		case DATATYPE_BOOL:
			snprintf(buf, VALUEAREA_SIZE, "%s", config->value.bdata ? "TRUE" : "FALSE");
			buttondot.setVisible(false);
			button0.setVisible(false);
			button1.setVisible(false);
			button2.setVisible(false);
			button3.setVisible(false);
			button4.setVisible(false);
			button5.setVisible(false);
			button6.setVisible(false);
			button7.setVisible(false);
			button8.setVisible(false);
			button9.setVisible(false);
			buttoncr.setVisible(false);
			buttonminus.setVisible(false);
			break;
		}

		*editbuf = '\0'; // Clear the buffer
		Unicode::strncpy(valueAreaBuffer, buf, VALUEAREA_SIZE);

		invalidate();
	}
}

void ConfigPopup::ok(const AbstractButton& b)
{
	if (config && *editbuf != '\0')
	{
		switch (config->type)
		{
		case DATATYPE_INT:
			config->value.idata = strtol(editbuf, NULL, 10);
			break;
		case DATATYPE_DOUBLE:
			config->value.ddata = strtod(editbuf, NULL);
			break;
		case DATATYPE_BOOL:
			config->value.bdata = (strcmp(editbuf, "TRUE") == 0);
			break;
		case DATATYPE_STRING:
			// TODO
			break;
		}
	}

	if (config && callback)
	{
		callback->execute(config, true);
	}

	this->callback = NULL;
	this->config = NULL;
	this->setVisible(false);
	invalidate();
}

void ConfigPopup::cancel(const AbstractButton& b)
{
	if (config && callback)
	{
		callback->execute(config, false);
	}
	this->callback = NULL;
	this->config = NULL;
	this->setVisible(false);
	invalidate();
}

void ConfigPopup::edit(const AbstractButton& b)
{
	if (config)
	{
		if (config->type == DATATYPE_BOOL)
		{
			if (&b == &upbutton)
			{
				strcpy(editbuf, "TRUE");
			}
			else if (&b == &downbutton)
			{
				strcpy(editbuf, "FALSE");
			}
		}
		else if (config->type == DATATYPE_INT || config->type == DATATYPE_DOUBLE)
		{
			if (&b == &buttoncr)
			{
				*editbuf = '\0';
			}
			else if (&b == &buttondot)
			{
				if (strchr(editbuf, '.') != NULL)
				{
					// dot already exists,
				}
				else
				{
					if (*editbuf == '\0')
					{
						// If empty, we also add a zero to it
						strcat(editbuf, "0.");
					}
					else
					{
						// Append dot
						strcat(editbuf, ".");
					}
				}
			}
			else if (&b == &buttonminus)
			{
				// Toggle minus sign
				if (strlen(editbuf) == 0)
				{
					strcat(editbuf, "-");
				}
				else if (editbuf[0] == '-')
				{
					// Remove minus sign
					for (char *p = editbuf; *p; p++)
						*p = *(p + 1);
				}
				else if (strlen(editbuf) < VALUEAREA_SIZE - 1)
				{
					// Insert minus sign if there is space
					char buf[sizeof(editbuf)];
					strcpy(buf, editbuf);
					editbuf[0] = '-';
					editbuf[1] = '\0';
					strcat(editbuf, buf);
				}
			}
			else if (&b == &upbutton || &b == &downbutton)
			{
				if (config->type == DATATYPE_INT)
				{
					// Obtain the value
					int value;
					if (*editbuf == '\0')
						value = config->value.idata;
					else
						value = strtol(editbuf, NULL, 10);
					if (&b == &upbutton)
						value++;
					else
						value--;
					snprintf(editbuf, VALUEAREA_SIZE, "%d", value);
				}
				else
				{
					// Obtain double value
					double value;
					if (*editbuf == '\0')
						value = config->value.ddata;
					else
						value = strtod(editbuf, NULL);
					if (value == 0)
					{
						if (&b == &upbutton)
						{
							value += 1;
						}
						else
						{
							value -= 1;
						}
					}
					else
					{
						double power = floor(log10(fabs(value))); // m*10^power, m>=1
						double increment = pow(10, power - 1); // Use the next significant digit for up/down
						if (&b == &upbutton)
						{
							value += increment;
						}
						else
						{
							value -= increment;
						}
					}
					snprintf(editbuf, VALUEAREA_SIZE, "%f", value);
				}
			}
			else
			{
				// Number buttons
				int num = -1;
				if (&b == &button0)
				{
					num = 0;
				}
				else if (&b == &button1)
				{
					num = 1;
				}
				else if (&b == &button2)
				{
					num = 2;
				}
				else if (&b == &button3)
				{
					num = 3;
				}
				else if (&b == &button4)
				{
					num = 4;
				}
				else if (&b == &button5)
				{
					num = 5;
				}
				else if (&b == &button6)
				{
					num = 6;
				}
				else if (&b == &button7)
				{
					num = 7;
				}
				else if (&b == &button8)
				{
					num = 8;
				}
				else if (&b == &button9)
				{
					num = 9;
				}

				if (num != -1)
				{
					if (num != 0 || strcmp(editbuf, "0") != 0)
					{
						// Do nothing if num==0 and editbuf=="0"
						// Append the char
						char s[2] = "0";
						s[0] += (char) num;

						if (strcmp(editbuf, "0") == 0)
						{
							// First meaningful digit
							strcpy(editbuf, s);
						}
						else
						{
							// Append
							strcat(editbuf, s);
						}
					}
				}
			}
		}

		// Truncate
		editbuf[VALUEAREA_SIZE - 1] = '\0';

		Unicode::strncpy(valueAreaBuffer, editbuf, VALUEAREA_SIZE);
		valueArea.invalidate();
	}
}

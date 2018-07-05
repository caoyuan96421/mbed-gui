#ifndef COORDINATEPOPUP_HPP

#define COORDINATEPOPUP_HPP

#include <gui_generated/containers/CoordinatePopupBase.hpp>
#include <NumericalWheelSelector.hpp>

class CoordinatePopup: public CoordinatePopupBase
{
public:

	typedef enum
	{
		FORMAT_D_M_S_SIGNED = 0, FORMAT_H_M_S_SIGNED = 1, FORMAT_D_M_S_NS = 2, FORMAT_H_M_S_UNSIGNED = 3,
	} CoordinateFormat;

	struct Coordinate
	{
		double v1;
		double v2;
		Coordinate(double v1, double v2) :
				v1(v1), v2(v2)
		{
		}
	};

	CoordinatePopup();
	virtual ~CoordinatePopup()
	{
	}

	void show(CoordinateFormat format1, CoordinateFormat format2, const char *name1, const char *name2);

	void setCallback(touchgfx::GenericCallback<Coordinate, bool> *cb)
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

	NumericalWheelSelector ws11, ws12, ws13;
	NumericalWheelSelector ws21, ws22, ws23;
	touchgfx::GenericCallback<Coordinate, bool> *callback;
	CoordinateFormat format[2];

	touchgfx::Callback<CoordinatePopup, const TextAreaWithOneWildcard &, const ClickEvent &> callbackNSClicked;
	touchgfx::Callback<CoordinatePopup, const TextAreaWithOneWildcard &, const ClickEvent &> callbackSignClicked;
	touchgfx::Callback<CoordinatePopup, const AbstractButton&> callbackok;
	touchgfx::Callback<CoordinatePopup, const AbstractButton&> callbackcancel;

	void nsClicked(const TextAreaWithOneWildcard &, const ClickEvent &);
	void signClicked(const TextAreaWithOneWildcard &, const ClickEvent &);
	virtual void ok(const AbstractButton &);
	virtual void cancel(const AbstractButton &);

};

#endif // COORDINATEPOPUP_HPP

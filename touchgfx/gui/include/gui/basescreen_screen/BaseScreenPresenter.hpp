#ifndef BASESCREEN_PRESENTER_HPP
#define BASESCREEN_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class BaseScreenView;

class BaseScreenPresenter : public Presenter, public ModelListener
{
public:
    BaseScreenPresenter(BaseScreenView& v);

    /**
     * The activate function is called automatically when this screen is "switched in"
     * (ie. made active). Initialization logic can be placed here.
     */
    virtual void activate();

    /**
     * The deactivate function is called automatically when this screen is "switched out"
     * (ie. made inactive). Teardown functionality can be placed here.
     */
    virtual void deactivate();

    virtual ~BaseScreenPresenter() {};


    void setTime(time_t timestamp);
	void setEqCoords(const EquatorialCoordinates &eq);

protected:
    BaseScreenView& view;

private:
    BaseScreenPresenter();

};


#endif // BASESCREEN_PRESENTER_HPP

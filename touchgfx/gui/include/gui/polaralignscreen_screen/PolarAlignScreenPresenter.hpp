#ifndef POLARALIGNSCREEN_PRESENTER_HPP
#define POLARALIGNSCREEN_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>
#include <gui/include/gui/basescreen_screen/BaseScreenPresenter.hpp>

using namespace touchgfx;

class PolarAlignScreenView;

class PolarAlignScreenPresenter : public BaseScreenPresenter
{
public:
    PolarAlignScreenPresenter(PolarAlignScreenView& v);

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

    virtual ~PolarAlignScreenPresenter() {};

private:
    PolarAlignScreenPresenter();

    PolarAlignScreenView& view;
};


#endif // POLARALIGNSCREEN_PRESENTER_HPP

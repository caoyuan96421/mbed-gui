#ifndef DUMMYSCREEN_PRESENTER_HPP
#define DUMMYSCREEN_PRESENTER_HPP

#include <gui/model/ModelListener.hpp>
#include <mvp/Presenter.hpp>

using namespace touchgfx;

class DummyScreenView;

class DummyScreenPresenter : public Presenter, public ModelListener
{
public:
    DummyScreenPresenter(DummyScreenView& v);

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

    virtual ~DummyScreenPresenter() {};

private:
    DummyScreenPresenter();

    DummyScreenView& view;
};


#endif // DUMMYSCREEN_PRESENTER_HPP

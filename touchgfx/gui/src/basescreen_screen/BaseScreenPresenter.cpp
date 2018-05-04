#include <gui/basescreen_screen/BaseScreenView.hpp>
#include <gui/basescreen_screen/BaseScreenPresenter.hpp>

BaseScreenPresenter::BaseScreenPresenter(BaseScreenView& v)
    : view(v)
{
}

void BaseScreenPresenter::activate()
{

}

void BaseScreenPresenter::deactivate()
{

}

void BaseScreenPresenter::setTime(time_t timestamp)
{
   	view.setTime(timestamp);
}

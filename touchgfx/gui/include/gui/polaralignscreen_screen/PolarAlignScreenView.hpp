#ifndef POLARALIGNSCREEN_VIEW_HPP
#define POLARALIGNSCREEN_VIEW_HPP

#include <gui_generated/polaralignscreen_screen/PolarAlignScreenViewBase.hpp>
#include <gui/polaralignscreen_screen/PolarAlignScreenPresenter.hpp>
#include <gui/BaseScreenAdaptor.h>

class PolarAlignScreenView : public PolarAlignScreenViewBase, public BaseScreenAdaptor
{
public:
    PolarAlignScreenView();
    virtual ~PolarAlignScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:
};

#endif // POLARALIGNSCREEN_VIEW_HPP

/*********************************************************************************/
/********** THIS FILE IS GENERATED BY TOUCHGFX DESIGNER, DO NOT MODIFY ***********/
/*********************************************************************************/
#ifndef POLARALIGNSCREEN_VIEW_BASE_HPP
#define POLARALIGNSCREEN_VIEW_BASE_HPP

#include <gui/common/FrontendApplication.hpp>
#include <mvp/View.hpp>
#include <gui/polaralignscreen_screen/PolarAlignScreenPresenter.hpp>
#include <touchgfx/widgets/Box.hpp>
#include <touchgfx/containers/ScrollableContainer.hpp>
#include <touchgfx/widgets/TextAreaWithWildcard.hpp>
#include <touchgfx/widgets/TextArea.hpp>
#include <touchgfx/widgets/ButtonWithLabel.hpp>
#include <touchgfx/widgets/ButtonWithIcon.hpp>

class PolarAlignScreenViewBase : public touchgfx::View<PolarAlignScreenPresenter>
{
public:
    PolarAlignScreenViewBase();
    virtual ~PolarAlignScreenViewBase() {}

protected:
    FrontendApplication& application() { 
        return *static_cast<FrontendApplication*>(Application::getInstance()); 
    }

    /*
     * Member Declarations
     */
    touchgfx::Box box1;
    touchgfx::ScrollableContainer scrollableContainer1;
    touchgfx::TextAreaWithOneWildcard text_description;
    touchgfx::TextArea textArea1;
    touchgfx::ButtonWithLabel buttonContinue;
    touchgfx::ButtonWithLabel buttonRestart;
    touchgfx::TextAreaWithOneWildcard textArea2;
    touchgfx::TextArea textArea3;
    touchgfx::TextArea textArea4;
    touchgfx::ButtonWithIcon buttonStop;
    touchgfx::ButtonWithLabel buttonAligned;
    touchgfx::TextArea textArea5;

    /*
     * Wildcard Buffers
     */
    static const uint16_t TEXT_DESCRIPTION_SIZE = 256;
    touchgfx::Unicode::UnicodeChar text_descriptionBuffer[TEXT_DESCRIPTION_SIZE];
    static const uint16_t TEXTAREA2_SIZE = 22;
    touchgfx::Unicode::UnicodeChar textArea2Buffer[TEXTAREA2_SIZE];

private:

};

#endif // POLARALIGNSCREEN_VIEW_BASE_HPP

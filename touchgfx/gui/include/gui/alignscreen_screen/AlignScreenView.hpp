#ifndef ALIGNSCREEN_VIEW_HPP
#define ALIGNSCREEN_VIEW_HPP

#include <gui_generated/alignscreen_screen/AlignScreenViewBase.hpp>
#include <gui/alignscreen_screen/AlignScreenPresenter.hpp>
#include <gui/BaseScreenAdaptor.h>
#include <touchgfx/Callback.hpp>
#include <gui/widgets/ButtonItem.h>
#include "StarCatalog.h"
#include "CelestialMath.h"

class AlignScreenView : public AlignScreenViewBase, public BaseScreenAdaptor
{
public:
    AlignScreenView();
    virtual ~AlignScreenView() {}
    virtual void setupScreen();
    virtual void tearDownScreen();
protected:

    void updateMenu();
    void updateCalibration();
    void buttonRefreshPressed(const AbstractButton&){
    	updateMenu();
    }
    void starSelected(const AbstractButton&);
    void buttonAddPressed(const AbstractButton&);
    void buttonDeletePressed(const AbstractButton&);
    void buttonGotoPressed(const AbstractButton&);
    void buttonAlignPressed(const AbstractButton&);
    void buttonStopPressed(const AbstractButton&);


	static void callback(StarInfo*, void*);
	void _callback(StarInfo *);

	void clearMenu(Container &);

	touchgfx::Callback<AlignScreenView, const AbstractButton&> buttonRefreshCallback;
	touchgfx::Callback<AlignScreenView, const AbstractButton&> buttonAddCallback;
	touchgfx::Callback<AlignScreenView, const AbstractButton&> buttonDeleteCallback;
	touchgfx::Callback<AlignScreenView, const AbstractButton&> buttonGotoCallback;
	touchgfx::Callback<AlignScreenView, const AbstractButton&> buttonAlignCallback;
	touchgfx::Callback<AlignScreenView, const AbstractButton&> buttonStopCallback;

	touchgfx::Callback<AlignScreenView, const AbstractButton&> starSelectedCallback;

	touchgfx::Container container_candidates;
	touchgfx::Container container_selected;

	time_t time_now;
	LocationCoordinates loc;

	float minAlt;
	int height;
	int num_candidates;
	int num_alignment;
	ButtonItem *selectedCandidate;
	ButtonItem *selectedAlignment;

	colortype defaultColor, selectedColor, pressedColor;
};

#endif // ALIGNSCREEN_VIEW_HPP

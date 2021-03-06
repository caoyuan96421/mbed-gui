/*********************************************************************************/
/********** THIS FILE IS GENERATED BY TOUCHGFX DESIGNER, DO NOT MODIFY ***********/
/*********************************************************************************/
#include <gui_generated/homescreen_screen/HomeScreenViewBase.hpp>
#include <touchgfx/Color.hpp>
#include "BitmapDatabase.hpp"
#include <texts/TextKeysAndLanguages.hpp>

HomeScreenViewBase::HomeScreenViewBase()  :
    buttonCallback(this, &HomeScreenViewBase::buttonCallbackHandler)
{
    box1.setPosition(0, 0, 480, 800);
    box1.setColor(touchgfx::Color::getColorFrom24BitRGB(0, 0, 0));

    mount_view.setPosition(23, 102, 200, 200);
    mount_view.setBitmaps(Bitmap(BITMAP_BUTTON1_ID), Bitmap(BITMAP_BUTTON1_PRESSED_ID));
    mount_view.setAction(buttonCallback);

    starmap_view.setPosition(260, 102, 200, 200);
    starmap_view.setBitmaps(Bitmap(BITMAP_BUTTON1_ID), Bitmap(BITMAP_BUTTON1_PRESSED_ID));
    starmap_view.setAction(buttonCallback);

    telescope1.setXY(71, 123);
    telescope1.setBitmap(Bitmap(BITMAP_TELESCOPE_ID));

    textArea1.setXY(65, 302);
    textArea1.setColor(touchgfx::Color::getColorFrom24BitRGB(176, 46, 46));
    textArea1.setLinespacing(0);
    textArea1.setTypedText(TypedText(T_SINGLEUSEID11));

    Image1.setXY(260, 102);
    Image1.setBitmap(Bitmap(BITMAP_STARS_ID));

    textArea1_1.setXY(280, 302);
    textArea1_1.setColor(touchgfx::Color::getColorFrom24BitRGB(176, 46, 46));
    textArea1_1.setLinespacing(0);
    textArea1_1.setTypedText(TypedText(T_SINGLEUSEID12));

    setting_view.setPosition(23, 404, 200, 200);
    setting_view.setBitmaps(Bitmap(BITMAP_BUTTON1_ID), Bitmap(BITMAP_BUTTON1_PRESSED_ID));
    setting_view.setAction(buttonCallback);

    textArea1_2.setXY(61, 604);
    textArea1_2.setColor(touchgfx::Color::getColorFrom24BitRGB(176, 46, 46));
    textArea1_2.setLinespacing(0);
    textArea1_2.setTypedText(TypedText(T_SINGLEUSEID13));

    image2.setXY(50, 431);
    image2.setBitmap(Bitmap(BITMAP_WRENCH_ID));

    utility_view.setPosition(260, 404, 200, 200);
    utility_view.setBitmaps(Bitmap(BITMAP_BUTTON1_ID), Bitmap(BITMAP_BUTTON1_PRESSED_ID));
    utility_view.setAction(buttonCallback);

    textArea1_2_1.setXY(287, 604);
    textArea1_2_1.setColor(touchgfx::Color::getColorFrom24BitRGB(176, 46, 46));
    textArea1_2_1.setLinespacing(0);
    textArea1_2_1.setTypedText(TypedText(T_SINGLEUSEID14));

    image3.setXY(260, 404);
    image3.setBitmap(Bitmap(BITMAP_UTILITY_ID));

    textArea1_2_2.setXY(91, 27);
    textArea1_2_2.setColor(touchgfx::Color::getColorFrom24BitRGB(56, 74, 186));
    textArea1_2_2.setLinespacing(0);
    textArea1_2_2.setTypedText(TypedText(T_SINGLEUSEID16));

    add(box1);
    add(mount_view);
    add(starmap_view);
    add(telescope1);
    add(textArea1);
    add(Image1);
    add(textArea1_1);
    add(setting_view);
    add(textArea1_2);
    add(image2);
    add(utility_view);
    add(textArea1_2_1);
    add(image3);
    add(textArea1_2_2);
}

void HomeScreenViewBase::buttonCallbackHandler(const touchgfx::AbstractButton& src)
{
    if (&src == &mount_view)
    {
        //Interaction1
        //When mount_view clicked change screen to MountScreen
        //Go to MountScreen with slide screen transition towards East
        application().gotoMountScreenScreenSlideTransitionEast();
    }
    else if (&src == &starmap_view)
    {
        //Interaction3
        //When starmap_view clicked change screen to StarMapScreen
        //Go to StarMapScreen with slide screen transition towards East
        application().gotoStarMapScreenScreenSlideTransitionEast();
    }
    else if (&src == &setting_view)
    {
        //Interaction2
        //When setting_view clicked change screen to SettingScreen
        //Go to SettingScreen with slide screen transition towards East
        application().gotoSettingScreenScreenSlideTransitionEast();
    }
    else if (&src == &utility_view)
    {
        //Interaction4
        //When utility_view clicked change screen to UtilityScreen
        //Go to UtilityScreen with slide screen transition towards East
        application().gotoUtilityScreenScreenSlideTransitionEast();
    }
}

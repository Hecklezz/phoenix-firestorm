/** 
 * @file llfloatereditextdaycycle.cpp
 * @brief Floater to create or edit a day cycle
 *
 * $LicenseInfo:firstyear=2011&license=viewerlgpl$
 * Second Life Viewer Source Code
 * Copyright (C) 2011, Linden Research, Inc.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * Linden Research, Inc., 945 Battery Street, San Francisco, CA  94111  USA
 * $/LicenseInfo$
 */

#include "llviewerprecompiledheaders.h"

#include "llfloatereditextdaycycle.h"

// libs
#include "llbutton.h"
#include "llcallbacklist.h"
#include "llcheckboxctrl.h"
#include "llcombobox.h"
#include "llloadingindicator.h"
#include "llmultisliderctrl.h"
#include "llnotifications.h"
#include "llnotificationsutil.h"
#include "llspinctrl.h"
#include "lltimectrl.h"
#include "lltabcontainer.h"
#include "llfilepicker.h"

#include "llsettingsvo.h"
#include "llinventorymodel.h"
#include "llviewerparcelmgr.h"

// newview
#include "llagent.h"
#include "llparcel.h"
#include "llflyoutcombobtn.h" //Todo: make a proper UI element/button/panel instead
#include "llregioninfomodel.h"
#include "llviewerregion.h"
#include "llpaneleditwater.h"
#include "llpaneleditsky.h"

#include "llenvironment.h"
#include "lltrans.h"

//=========================================================================
namespace {
    const std::string track_tabs[] = {
        "water_track",
        "sky1_track",
        "sky2_track",
        "sky3_track",
        "sky4_track",
    };

    // For flyout
    const std::string XML_FLYOUTMENU_FILE("menu_save_settings.xml");
    // From menu_save_settings.xml, consider moving into flyout since it should be supported by flyout either way
    const std::string ACTION_SAVE("save_settings");
    const std::string ACTION_SAVEAS("save_as_new_settings");
    const std::string ACTION_APPLY_LOCAL("apply_local");
    const std::string ACTION_APPLY_PARCEL("apply_parcel");
    const std::string ACTION_APPLY_REGION("apply_region");

    const F32 DAY_CYCLE_PLAY_TIME_SECONDS = 60;

    const F32 FRAME_SLOP_FACTOR = 0.025f;
}

//=========================================================================
const std::string LLFloaterEditExtDayCycle::KEY_INVENTORY_ID("inventory_id");
const std::string LLFloaterEditExtDayCycle::KEY_LIVE_ENVIRONMENT("live_environment");
const std::string LLFloaterEditExtDayCycle::KEY_DAY_LENGTH("day_length");

//=========================================================================
LLFloaterEditExtDayCycle::LLFloaterEditExtDayCycle(const LLSD &key) :
    LLFloater(key),
    mFlyoutControl(NULL),
    mCancelButton(NULL),
    mDayLength(0),
    mCurrentTrack(4),
    mTimeSlider(NULL),
    mFramesSlider(NULL),
    mCurrentTimeLabel(NULL),
    mImportButton(nullptr),
    mInventoryId(),
    mInventoryItem(nullptr),
    mSkyBlender(),
    mWaterBlender(),
    mScratchSky(),
    mScratchWater(),
    mIsPlaying(false)
{

    mCommitCallbackRegistrar.add("DayCycle.Track", [this](LLUICtrl *ctrl, const LLSD &data) { onTrackSelectionCallback(data); });
    mCommitCallbackRegistrar.add("DayCycle.PlayActions", [this](LLUICtrl *ctrl, const LLSD &data) { onPlayActionCallback(data); });

    mScratchSky = LLSettingsVOSky::buildDefaultSky();
    mScratchWater = LLSettingsVOWater::buildDefaultWater();
}

LLFloaterEditExtDayCycle::~LLFloaterEditExtDayCycle()
{
    // Todo: consider remaking mFlyoutControl into full view class that initializes intself with floater,
    // complete with postbuild, e t c...
    delete mFlyoutControl;
}

// virtual
BOOL LLFloaterEditExtDayCycle::postBuild()
{
    getChild<LLLineEditor>("day_cycle_name")->setKeystrokeCallback(boost::bind(&LLFloaterEditExtDayCycle::onCommitName, this, _1, _2), NULL);

    mCancelButton = getChild<LLButton>("cancel_btn", true);
    mAddFrameButton = getChild<LLButton>("add_frame", true);
    mDeleteFrameButton = getChild<LLButton>("delete_frame", true);
    mTimeSlider = getChild<LLMultiSliderCtrl>("WLTimeSlider");
    mFramesSlider = getChild<LLMultiSliderCtrl>("WLDayCycleFrames");
    mSkyTabLayoutContainer = getChild<LLView>("frame_settings_sky", true);
    mWaterTabLayoutContainer = getChild<LLView>("frame_settings_water", true);
    mCurrentTimeLabel = getChild<LLTextBox>("current_time", true);
    mImportButton = getChild<LLButton>("btn_import", true);

    mFlyoutControl = new LLFlyoutComboBtnCtrl(this, "save_btn", "btn_flyout", XML_FLYOUTMENU_FILE);
    mFlyoutControl->setAction([this](LLUICtrl *ctrl, const LLSD &data) { onButtonApply(ctrl, data); });

    mCancelButton->setCommitCallback([this](LLUICtrl *ctrl, const LLSD &data) { onBtnCancel(); });
    mTimeSlider->setCommitCallback([this](LLUICtrl *ctrl, const LLSD &data) { onTimeSliderMoved(); });
    mAddFrameButton->setCommitCallback([this](LLUICtrl *ctrl, const LLSD &data) { onAddTrack(); });
    mDeleteFrameButton->setCommitCallback([this](LLUICtrl *ctrl, const LLSD &data) { onRemoveTrack(); });
    mImportButton->setCommitCallback([this](LLUICtrl *, const LLSD &){ onButtonImport(); });

    mFramesSlider->setCommitCallback([this](LLUICtrl *, const LLSD &data) { onFrameSliderCallback(data); });
    mFramesSlider->setDoubleClickCallback([this](LLUICtrl*, S32 x, S32 y, MASK mask){ onFrameSliderDoubleClick(x, y, mask); });
    mFramesSlider->setMouseDownCallback([this](LLUICtrl*, S32 x, S32 y, MASK mask){ onFrameSliderMouseDown(x, y, mask); });
    mFramesSlider->setMouseUpCallback([this](LLUICtrl*, S32 x, S32 y, MASK mask){ onFrameSliderMouseUp(x, y, mask); });

    mTimeSlider->addSlider(0);

    //getChild<LLButton>("sky1_track", true)->setToggleState(true);

	return TRUE;
}

void LLFloaterEditExtDayCycle::onOpen(const LLSD& key)
{
    LLEnvironment::instance().setSelectedEnvironment(LLEnvironment::ENV_EDIT);
    LLEnvironment::instance().updateEnvironment();

    mEditingEnv = LLEnvironment::ENV_NONE;
    mEditDay.reset();
    if (key.has(KEY_INVENTORY_ID))
    {
        loadInventoryItem(key[KEY_INVENTORY_ID].asUUID());
    }
    else if (key.has(KEY_LIVE_ENVIRONMENT))
    {
        LLEnvironment::EnvSelection_t env = static_cast<LLEnvironment::EnvSelection_t>(key[KEY_LIVE_ENVIRONMENT].asInteger());

        loadLiveEnvironment(env);
    }
    else
    {
        loadLiveEnvironment(LLEnvironment::ENV_DEFAULT);
    }

    mDayLength.value(0);
    if (key.has(KEY_DAY_LENGTH))
    {
        mDayLength.value(key[KEY_DAY_LENGTH].asReal());
    }

    // time labels
    mCurrentTimeLabel->setTextArg("[PRCNT]", std::string("0"));
    const S32 max_elm = 5;
    if (mDayLength.value() != 0)
    {
        S32Hours hrs;
        S32Minutes minutes;
        LLSettingsDay::Seconds total;
        LLUIString formatted_label = getString("time_label");
        for (int i = 0; i < max_elm; i++)
        {
            total = ((mDayLength / (max_elm - 1)) * i); 
            hrs = total;
            minutes = total - hrs;

            formatted_label.setArg("[HH]", llformat("%d", hrs.value()));
            formatted_label.setArg("[MM]", llformat("%d", abs(minutes.value())));
            getChild<LLTextBox>("p" + llformat("%d", i), true)->setTextArg("[DSC]", formatted_label.getString());
        }
        hrs = mDayLength;
        minutes = mDayLength - hrs;
        formatted_label.setArg("[HH]", llformat("%d", hrs.value()));
        formatted_label.setArg("[MM]", llformat("%d", abs(minutes.value())));
        mCurrentTimeLabel->setTextArg("[DSC]", formatted_label.getString());
    }
    else
    {
        for (int i = 0; i < max_elm; i++)
        {
            getChild<LLTextBox>("p" + llformat("%d", i), true)->setTextArg("[DSC]", std::string());
        }
        mCurrentTimeLabel->setTextArg("[DSC]", std::string());
    }

    const LLEnvironment::altitude_list_t &altitudes = LLEnvironment::instance().getRegionAltitudes();

    for (S32 idx = 1; idx < 4; ++idx)
    {
        std::stringstream label;
        label << altitudes[idx] << "m";
        getChild<LLButton>(track_tabs[idx + 1], true)->setTextArg("[DSC]", label.str());
    }

}

void LLFloaterEditExtDayCycle::onClose(bool app_quitting)
{
    // there's no point to change environment if we're quitting
    // or if we already restored environment
    if (!app_quitting && LLEnvironment::instance().getSelectedEnvironment() == LLEnvironment::ENV_EDIT)
    {
        LLEnvironment::instance().setSelectedEnvironment(LLEnvironment::ENV_LOCAL);
    }
    stopPlay();
}

void LLFloaterEditExtDayCycle::onVisibilityChange(BOOL new_visibility)
{
    if (new_visibility)
    {
        LLEnvironment::instance().setEnvironment(LLEnvironment::ENV_EDIT, mScratchSky, mScratchWater);
        LLEnvironment::instance().setSelectedEnvironment(LLEnvironment::ENV_EDIT);
    }
    else
    {
        LLEnvironment::instance().setSelectedEnvironment(LLEnvironment::ENV_LOCAL);
        stopPlay();
    }
}

void LLFloaterEditExtDayCycle::refresh()
{
    if (mEditDay)
    {
        LLLineEditor* name_field = getChild<LLLineEditor>("day_cycle_name");
        name_field->setText(mEditDay->getName());
    }

    bool is_inventory_avail = canUseInventory();

    mFlyoutControl->setMenuItemEnabled(ACTION_SAVE, is_inventory_avail);
    mFlyoutControl->setMenuItemEnabled(ACTION_SAVEAS, is_inventory_avail);


    LLFloater::refresh();
}


void LLFloaterEditExtDayCycle::onButtonApply(LLUICtrl *ctrl, const LLSD &data)
{
    std::string ctrl_action = ctrl->getName();

    if (ctrl_action == ACTION_SAVE)
    {
        doApplyUpdateInventory();
    }
    else if (ctrl_action == ACTION_SAVEAS)
    {
        doApplyCreateNewInventory();
    }
    else if ((ctrl_action == ACTION_APPLY_LOCAL) ||
        (ctrl_action == ACTION_APPLY_PARCEL) ||
        (ctrl_action == ACTION_APPLY_REGION))
    {
        doApplyEnvironment(ctrl_action);
    }
    else
    {
        LL_WARNS("ENVIRONMENT") << "Unknown settings action '" << ctrl_action << "'" << LL_ENDL;
    }
}

void LLFloaterEditExtDayCycle::onBtnCancel()
{
    closeFloater(); // will restore env
}

void LLFloaterEditExtDayCycle::onButtonImport()
{
    doImportFromDisk();
}

void LLFloaterEditExtDayCycle::onAddTrack()
{
    // todo: 2.5% safety zone
    std::string sldr_key = mFramesSlider->getCurSlider();
    F32 frame = mTimeSlider->getCurSliderValue();
    LLSettingsBase::ptr_t setting;
    if (mEditDay->getSettingsAtKeyframe(frame, mCurrentTrack).get() != NULL)
    {
        return;
    }

    if (mCurrentTrack == LLSettingsDay::TRACK_WATER)
    {
        // scratch water should always have the current water settings.
        setting = mScratchWater->buildClone();
        mEditDay->setWaterAtKeyframe(std::dynamic_pointer_cast<LLSettingsWater>(setting), frame);
    }
    else
    {
        // scratch sky should always have the current sky settings.
        setting = mScratchSky->buildClone();
        mEditDay->setSkyAtKeyframe(std::dynamic_pointer_cast<LLSettingsSky>(setting), frame, mCurrentTrack);
    }

    addSliderFrame(frame, setting);
    updateTabs();
}

void LLFloaterEditExtDayCycle::onRemoveTrack()
{
    std::string sldr_key = mFramesSlider->getCurSlider();
    if (!sldr_key.empty())
    {
        return;
    }
    removeCurrentSliderFrame();
    updateButtons();
}

void LLFloaterEditExtDayCycle::onCommitName(class LLLineEditor* caller, void* user_data)
{
    mEditDay->setName(caller->getText());
}

void LLFloaterEditExtDayCycle::onTrackSelectionCallback(const LLSD& user_data)
{
    U32 track_index = user_data.asInteger(); // 1-5
    selectTrack(track_index);
}

void LLFloaterEditExtDayCycle::onPlayActionCallback(const LLSD& user_data)
{
    std::string action = user_data.asString();
    F32 frame = mTimeSlider->getCurSliderValue();
    if (action == "play")
    {
        startPlay();
    }
    else if (action == "pause")
    {
        stopPlay();
    }
    else if (mSliderKeyMap.size() != 0)
    {
        F32 new_frame = 0;
        if (action == "forward")
        {
            new_frame = mEditDay->getUpperBoundFrame(mCurrentTrack, frame);
        }
        else if (action == "back")
        {
            new_frame = mEditDay->getLowerBoundFrame(mCurrentTrack, frame - (mTimeSlider->getIncrement() / 2));
        }
        selectFrame(new_frame);
        stopPlay();
    }
}

void LLFloaterEditExtDayCycle::onFrameSliderCallback(const LLSD &data)
{
    //LL_WARNS("LAPRAS") << "LLFloaterEditExtDayCycle::onFrameSliderCallback(" << data << ")" << LL_ENDL;

    std::string curslider = mFramesSlider->getCurSlider();

    LL_WARNS("LAPRAS") << "Current slider set to \"" << curslider << "\"" << LL_ENDL;
    F32 sliderpos(0.0);

    if (curslider.empty())
    {
        S32 x(0), y(0);
        LLUI::getMousePositionLocal(mFramesSlider, &x, &y);

        sliderpos = mFramesSlider->getSliderValueFromX(x);
    }
    else
    {
        sliderpos = mFramesSlider->getCurSliderValue();
    }

    mTimeSlider->setCurSliderValue(sliderpos);
//     if (mSliderKeyMap.size() == 0)
//     {
//         mLastFrameSlider.clear();
//         return;
//     }
//     // make sure we have a slider
//     const std::string& cur_sldr = mFramesSlider->getCurSlider();
//     if (cur_sldr.empty())
//     {
//         mLastFrameSlider.clear();
//         return;
//     }
// 
//     F32 new_frame = mFramesSlider->getCurSliderValue();
//     // todo: add safety 2.5% checks
//     keymap_t::iterator iter = mSliderKeyMap.find(cur_sldr);
//     if (iter != mSliderKeyMap.end() && mEditDay->getSettingsAtKeyframe(new_frame, mCurrentTrack).get() == NULL)
//     {
//         if (gKeyboard->currentMask(TRUE) == MASK_SHIFT)
//         {
//             LL_DEBUGS() << "Copying frame from " << iter->second.mFrame << " to " << new_frame << LL_ENDL;
//             LLSettingsBase::ptr_t new_settings;
// 
//             // mEditDay still remembers old position, add copy at new position
//             if (mCurrentTrack == LLSettingsDay::TRACK_WATER)
//             {
//                 LLSettingsWaterPtr_t water_ptr = std::dynamic_pointer_cast<LLSettingsWater>(iter->second.pSettings)->buildClone();
//                 mEditDay->setWaterAtKeyframe(water_ptr, new_frame);
//                 new_settings = water_ptr;
//             }
//             else
//             {
//                 LLSettingsSkyPtr_t sky_ptr = std::dynamic_pointer_cast<LLSettingsSky>(iter->second.pSettings)->buildClone();
//                 mEditDay->setSkyAtKeyframe(sky_ptr, new_frame, mCurrentTrack);
//                 new_settings = sky_ptr;
//             }
// 
//             // mSliderKeyMap still remembers old position, for simplicity, just move it to be identical to slider
//             F32 old_frame = iter->second.mFrame;
//             iter->second.mFrame = new_frame;
//             // slider already moved old frame, create new one in old place
//             addSliderFrame(old_frame, new_settings, false /*because we are going to reselect new one*/);
//             // reselect new frame
//             mFramesSlider->setCurSlider(iter->first);
//         }
//         else
//         {
//             LL_DEBUGS() << "Moving frame from " << iter->second.mFrame << " to " << new_frame << LL_ENDL;
//             if (mEditDay->moveTrackKeyframe(mCurrentTrack, iter->second.mFrame, new_frame))
//             {
//                 iter->second.mFrame = new_frame;
//             }
//         }
//     }
// 
//     mTimeSlider->setCurSliderValue(new_frame);
// 
//     if (mLastFrameSlider != cur_sldr)
//     {
//         // technically should not be possible for both frame and slider to change
//         // but for safety, assume that they can change independently and both
//         mLastFrameSlider = cur_sldr;
//         updateTabs();
//     }
//     else
//     {
//         updateButtons();
//         updateTimeAndLabel();
//     }
}

void LLFloaterEditExtDayCycle::onFrameSliderDoubleClick(S32 x, S32 y, MASK mask)
{
    onAddTrack();
}

void LLFloaterEditExtDayCycle::onFrameSliderMouseDown(S32 x, S32 y, MASK mask)
{
    stopPlay();
    F32 sliderpos = mFramesSlider->getSliderValueFromX(x);

    std::string slidername = mFramesSlider->getCurSlider();

    if (!slidername.empty())
    {
        F32 sliderval = mFramesSlider->getSliderValue(slidername);

        LL_WARNS("LAPRAS") << "Selected vs mouse delta = " << (sliderval - sliderpos) << LL_ENDL;

        if (fabs(sliderval - sliderpos) > FRAME_SLOP_FACTOR)
        {
            mFramesSlider->resetCurSlider();
        }
    }
    LL_WARNS("LAPRAS") << "DOWN: X=" << x << "  Y=" << y << " MASK=" << mask << " Position=" << sliderpos << LL_ENDL;
}

void LLFloaterEditExtDayCycle::onFrameSliderMouseUp(S32 x, S32 y, MASK mask)
{
    F32 sliderpos = mFramesSlider->getSliderValueFromX(x);

    LL_WARNS("LAPRAS") << "  UP: X=" << x << "  Y=" << y << " MASK=" << mask << " Position=" << sliderpos << LL_ENDL;
    mTimeSlider->setCurSliderValue(sliderpos);
    selectFrame(sliderpos);
}

void LLFloaterEditExtDayCycle::onTimeSliderMoved()
{
    selectFrame(mTimeSlider->getCurSliderValue());
}

void LLFloaterEditExtDayCycle::selectTrack(U32 track_index, bool force )
{
    mCurrentTrack = track_index;
    LLButton* button = getChild<LLButton>(track_tabs[track_index], true);
    if (button->getToggleState() && !force)
    {
        return;
    }

    for (int i = 0; i < LLSettingsDay::TRACK_MAX; i++) // use max value
    {
        getChild<LLButton>(track_tabs[i], true)->setToggleState(i == track_index);
    }

    bool show_water = (mCurrentTrack == LLSettingsDay::TRACK_WATER);
    mSkyTabLayoutContainer->setVisible(!show_water);
    mWaterTabLayoutContainer->setVisible(show_water);
    updateSlider();
}

void LLFloaterEditExtDayCycle::selectFrame(F32 frame)
{
    mFramesSlider->resetCurSlider();


    keymap_t::iterator iter = mSliderKeyMap.begin();
    keymap_t::iterator end_iter = mSliderKeyMap.end();
    while (iter != end_iter)
    {
        if (fabs(iter->second.mFrame - frame) <= FRAME_SLOP_FACTOR)
        {
            mFramesSlider->setCurSlider(iter->first);
            frame = iter->second.mFrame;  
            break;
        }
        iter++;
    }

    mTimeSlider->setCurSliderValue(frame);
    // block or update tabs according to new selection
    updateTabs();
}

void LLFloaterEditExtDayCycle::clearTabs()
{
    // Note: If this doesn't look good, init panels with default settings. It might be better looking
    if (mCurrentTrack == LLSettingsDay::TRACK_WATER)
    {
        const LLSettingsWaterPtr_t p_water = LLSettingsWaterPtr_t(NULL);
        updateWaterTabs(p_water);
    }
    else
    {
        const LLSettingsSkyPtr_t p_sky = LLSettingsSkyPtr_t(NULL);
        updateSkyTabs(p_sky);
    }
    updateButtons();
    updateTimeAndLabel();
}

void LLFloaterEditExtDayCycle::updateTabs()
{
    reblendSettings();
    syncronizeTabs();

    updateButtons();
    updateTimeAndLabel();
}

void LLFloaterEditExtDayCycle::updateWaterTabs(const LLSettingsWaterPtr_t &p_water)
{
    LLView* tab_container = mWaterTabLayoutContainer->getChild<LLView>("water_tabs"); //can't extract panels directly, since it is in 'tuple'
    LLPanelSettingsWaterMainTab* panel = dynamic_cast<LLPanelSettingsWaterMainTab*>(tab_container->getChildView("water_panel"));
    if (panel)
    {
        panel->setWater(p_water);
    }
}

void LLFloaterEditExtDayCycle::updateSkyTabs(const LLSettingsSkyPtr_t &p_sky)
{
    LLView* tab_container = mSkyTabLayoutContainer->getChild<LLView>("sky_tabs"); //can't extract panels directly, since they are in 'tuple'

    LLPanelSettingsSky* panel;
    panel = dynamic_cast<LLPanelSettingsSky*>(tab_container->getChildView("atmosphere_panel"));
    if (panel)
    {
        panel->setSky(p_sky);
    }
    panel = dynamic_cast<LLPanelSettingsSky*>(tab_container->getChildView("clouds_panel"));
    if (panel)
    {
        panel->setSky(p_sky);
    }
    panel = dynamic_cast<LLPanelSettingsSky*>(tab_container->getChildView("moon_panel"));
    if (panel)
    {
        panel->setSky(p_sky);
    }
}

void LLFloaterEditExtDayCycle::setWaterTabsEnabled(BOOL enable)
{
    LLView* tab_container = mWaterTabLayoutContainer->getChild<LLView>("water_tabs"); //can't extract panels directly, since it is in 'tuple'
    LLPanelSettingsWaterMainTab* panel = dynamic_cast<LLPanelSettingsWaterMainTab*>(tab_container->getChildView("water_panel"));
    if (panel)
    {
        panel->setEnabled(enable);
        panel->setAllChildrenEnabled(enable);
    }
}

void LLFloaterEditExtDayCycle::setSkyTabsEnabled(BOOL enable)
{
    LLView* tab_container = mSkyTabLayoutContainer->getChild<LLView>("sky_tabs"); //can't extract panels directly, since they are in 'tuple'

    LLPanelSettingsSky* panel;
    panel = dynamic_cast<LLPanelSettingsSky*>(tab_container->getChildView("atmosphere_panel"));
    if (panel)
    {
        panel->setEnabled(enable);
        panel->setAllChildrenEnabled(enable);
    }
    panel = dynamic_cast<LLPanelSettingsSky*>(tab_container->getChildView("clouds_panel"));
    if (panel)
    {
        panel->setEnabled(enable);
        panel->setAllChildrenEnabled(enable);
    }
    panel = dynamic_cast<LLPanelSettingsSky*>(tab_container->getChildView("moon_panel"));
    if (panel)
    {
        panel->setEnabled(enable);
        panel->setAllChildrenEnabled(enable);
    }
}

void LLFloaterEditExtDayCycle::updateButtons()
{
    F32 frame = mTimeSlider->getCurSliderValue();
    LLSettingsBase::ptr_t settings = mEditDay->getSettingsAtKeyframe(frame, mCurrentTrack);
    bool can_add = settings.get() == NULL;
    mAddFrameButton->setEnabled(can_add);
    mDeleteFrameButton->setEnabled(!can_add);
}

void LLFloaterEditExtDayCycle::updateSlider()
{
    F32 frame_position = mTimeSlider->getCurSliderValue();
    mFramesSlider->clear();
    mSliderKeyMap.clear();

    LLSettingsDay::CycleTrack_t track = mEditDay->getCycleTrack(mCurrentTrack);
    for (auto &track_frame : track)
    {
        addSliderFrame(track_frame.first, track_frame.second, false);
    }

    if (mSliderKeyMap.size() > 0)
    {
        // update positions
        mLastFrameSlider = mFramesSlider->getCurSlider();
        updateTabs();
    }
    else
    {
        // disable panels
        clearTabs();
        mLastFrameSlider.clear();
    }

    selectFrame(frame_position);
}

void LLFloaterEditExtDayCycle::updateTimeAndLabel()
{
    F32 time = mTimeSlider->getCurSliderValue();
    mCurrentTimeLabel->setTextArg("[PRCNT]", llformat("%.0f", time * 100));
    if (mDayLength.value() != 0)
    {
        LLUIString formatted_label = getString("time_label");

        LLSettingsDay::Seconds total = (mDayLength  * time); 
        S32Hours hrs = total;
        S32Minutes minutes = total - hrs;

        formatted_label.setArg("[HH]", llformat("%d", hrs.value()));
        formatted_label.setArg("[MM]", llformat("%d", abs(minutes.value())));
        mCurrentTimeLabel->setTextArg("[DSC]", formatted_label.getString());
    }
    else
    {
        mCurrentTimeLabel->setTextArg("[DSC]", std::string());
    }

    // Update blender here:
}

void LLFloaterEditExtDayCycle::addSliderFrame(const F32 frame, LLSettingsBase::ptr_t &setting, bool update_ui)
{
    // multi slider distinguishes elements by key/name in string format
    // store names to map to be able to recall dependencies
    std::string new_slider = mFramesSlider->addSlider(frame);
    mSliderKeyMap[new_slider] = FrameData(frame, setting);

    if (update_ui)
    {
        mLastFrameSlider = new_slider;
        mTimeSlider->setCurSliderValue(frame);
        updateTabs();
    }
}

void LLFloaterEditExtDayCycle::removeCurrentSliderFrame()
{
    std::string sldr = mFramesSlider->getCurSlider();
    if (sldr.empty())
    {
        return;
    }
    mFramesSlider->deleteCurSlider();
    keymap_t::iterator iter = mSliderKeyMap.find(sldr);
    if (iter != mSliderKeyMap.end())
    {
        LL_DEBUGS() << "Removing frame from " << iter->second.mFrame << LL_ENDL;
        mSliderKeyMap.erase(iter);
        mEditDay->removeTrackKeyframe(mCurrentTrack, iter->second.mFrame);
    }

    mLastFrameSlider = mFramesSlider->getCurSlider();
    mTimeSlider->setCurSliderValue(mFramesSlider->getCurSliderValue());
    updateTabs();
}

//-------------------------------------------------------------------------

LLFloaterEditExtDayCycle::connection_t LLFloaterEditExtDayCycle::setEditCommitSignal(LLFloaterEditExtDayCycle::edit_commit_signal_t::slot_type cb)
{
    return mCommitSignal.connect(cb);
}

void LLFloaterEditExtDayCycle::loadInventoryItem(const LLUUID  &inventoryId)
{
    if (inventoryId.isNull())
    {
        LL_WARNS("SETTINGS") << "Attempt to load NULL inventory ID" << LL_ENDL;
        mInventoryItem = nullptr;
        mInventoryId.setNull();
        return;
    }

    mInventoryId = inventoryId;
    LL_INFOS("SETTINGS") << "Setting edit inventory item to " << mInventoryId << "." << LL_ENDL;
    mInventoryItem = gInventory.getItem(mInventoryId);

    if (!mInventoryItem)
    {
        LL_WARNS("SETTINGS") << "Could not find inventory item with Id = " << mInventoryId << LL_ENDL;
        mInventoryId.setNull();
        mInventoryItem = nullptr;
        return;
    }

    if (mInventoryItem->getAssetUUID().isNull())
    {
        LL_WARNS("SETTINGS") << "Asset ID in inventory item is NULL (" << mInventoryId << ")" <<  LL_ENDL;
        mInventoryId.setNull();
        mInventoryItem = nullptr;
        return;
    }

    LLSettingsVOBase::getSettingsAsset(mInventoryItem->getAssetUUID(),
        [this](LLUUID asset_id, LLSettingsBase::ptr_t settins, S32 status, LLExtStat) { onAssetLoaded(asset_id, settins, status); });
}

void LLFloaterEditExtDayCycle::onAssetLoaded(LLUUID asset_id, LLSettingsBase::ptr_t settings, S32 status)
{
    if (!settings || status)
    {
        LLSD args;
        args["DESC"] = (mInventoryItem) ? mInventoryItem->getName() : "Unknown";
        LLNotificationsUtil::add("FailedToFindSettings", args);
        closeFloater();
        return;
    }
    mEditDay = std::dynamic_pointer_cast<LLSettingsDay>(settings);
    updateEditEnvironment();
    syncronizeTabs();
    refresh();
}

void LLFloaterEditExtDayCycle::loadLiveEnvironment(LLEnvironment::EnvSelection_t env)
{
    mEditingEnv = env;
    for (S32 idx = static_cast<S32>(env); idx <= LLEnvironment::ENV_DEFAULT; ++idx)
    {
        LLSettingsDay::ptr_t day = LLEnvironment::instance().getEnvironmentDay(static_cast<LLEnvironment::EnvSelection_t>(idx));

        if (day)
        {
            mEditDay = day->buildClone();
            break;
        }
    }

    if (!mEditDay)
    {
        LL_WARNS("SETTINGS") << "Unable to load environment " << env << " building default." << LL_ENDL;
        mEditDay = LLSettingsVODay::buildDefaultDayCycle();
    }

    updateEditEnvironment();
    syncronizeTabs();
    refresh();
}

void LLFloaterEditExtDayCycle::updateEditEnvironment(void)
{

    S32 skytrack = (mCurrentTrack) ? mCurrentTrack : 1;
    mSkyBlender = std::make_shared<LLTrackBlenderLoopingManual>(mScratchSky, mEditDay, skytrack);
    mWaterBlender = std::make_shared<LLTrackBlenderLoopingManual>(mScratchWater, mEditDay, LLSettingsDay::TRACK_WATER);

    selectTrack(1, true);

    reblendSettings();

    LLEnvironment::instance().setEnvironment(LLEnvironment::ENV_EDIT, mScratchSky, mScratchWater);
}

void LLFloaterEditExtDayCycle::syncronizeTabs()
{
    // This should probably get moved into "updateTabs"

    F32 frame = mTimeSlider->getCurSliderValue();
    bool canedit(false);

    LLSettingsWater::ptr_t psettingWater;
    LLTabContainer * tabs = mWaterTabLayoutContainer->getChild<LLTabContainer>("water_tabs");
    if (mCurrentTrack == LLSettingsDay::TRACK_WATER)
    {
        canedit = true;
        psettingWater = std::static_pointer_cast<LLSettingsWater>(mEditDay->getSettingsAtKeyframe(frame, LLSettingsDay::TRACK_WATER));
        if (!psettingWater)
        {
            canedit = false;
            psettingWater = mScratchWater;
        }
    }
    else 
    {
        psettingWater = mScratchWater;
    }

    S32 count = tabs->getTabCount();
    for (S32 idx = 0; idx < count; ++idx)
    {
        LLSettingsEditPanel *panel = static_cast<LLSettingsEditPanel *>(tabs->getPanelByIndex(idx));
        if (panel)
        {
            panel->setSettings(psettingWater);
            panel->setEnabled(canedit);
            panel->setAllChildrenEnabled(canedit);
            panel->refresh();
        }
    }

    LLSettingsSky::ptr_t psettingSky;

    canedit = false;
    tabs = mSkyTabLayoutContainer->getChild<LLTabContainer>("sky_tabs");
    if (mCurrentTrack != LLSettingsDay::TRACK_WATER)
    {
        canedit = true;
        psettingSky = std::static_pointer_cast<LLSettingsSky>(mEditDay->getSettingsAtKeyframe(frame, mCurrentTrack));
        if (!psettingSky)
        {
            canedit = false;
            psettingSky = mScratchSky;
        }
    }
    else
    {
        psettingSky = mScratchSky;
    }

    count = tabs->getTabCount();
    for (S32 idx = 0; idx < count; ++idx)
    {
        LLSettingsEditPanel *panel = static_cast<LLSettingsEditPanel *>(tabs->getPanelByIndex(idx));
        if (panel)
        {
            panel->setSettings(psettingSky);
            panel->setEnabled(canedit);
            panel->setAllChildrenEnabled(canedit);
            panel->refresh();
        }
    }

    LLEnvironment::instance().setEnvironment(LLEnvironment::ENV_EDIT, psettingSky, psettingWater);
}

void LLFloaterEditExtDayCycle::reblendSettings()
{
    F64 position = mTimeSlider->getCurSliderValue();

    if ((mSkyBlender->getTrack() != mCurrentTrack) && (mCurrentTrack != LLSettingsDay::TRACK_WATER))
    {
        mSkyBlender->switchTrack(mCurrentTrack, position);
    }
    else
        mSkyBlender->setPosition(position);

    mWaterBlender->setPosition(position);    
}

void LLFloaterEditExtDayCycle::doApplyCreateNewInventory()
{
    // This method knows what sort of settings object to create.
    LLSettingsVOBase::createInventoryItem(mEditDay, [this](LLUUID asset_id, LLUUID inventory_id, LLUUID, LLSD results) { onInventoryCreated(asset_id, inventory_id, results); });
}

void LLFloaterEditExtDayCycle::doApplyUpdateInventory()
{
    if (mInventoryId.isNull())
        LLSettingsVOBase::createInventoryItem(mEditDay, [this](LLUUID asset_id, LLUUID inventory_id, LLUUID, LLSD results) { onInventoryCreated(asset_id, inventory_id, results); });
    else
        LLSettingsVOBase::updateInventoryItem(mEditDay, mInventoryId, [this](LLUUID asset_id, LLUUID inventory_id, LLUUID, LLSD results) { onInventoryUpdated(asset_id, inventory_id, results); });
}

void LLFloaterEditExtDayCycle::doApplyEnvironment(const std::string &where)
{
    LLEnvironment::EnvSelection_t env(LLEnvironment::ENV_DEFAULT);
    bool updateSimulator(where != ACTION_APPLY_LOCAL);

    if (where == ACTION_APPLY_LOCAL)
        env = LLEnvironment::ENV_LOCAL;
    else if (where == ACTION_APPLY_PARCEL)
        env = LLEnvironment::ENV_PARCEL;
    else if (where == ACTION_APPLY_REGION)
        env = LLEnvironment::ENV_REGION;
    else
    {
        LL_WARNS("ENVIRONMENT") << "Unknown apply '" << where << "'" << LL_ENDL;
        return;
    }

    LLEnvironment::instance().setEnvironment(env, mEditDay);
    if (updateSimulator)
    {
        LL_WARNS("ENVIRONMENT") << "Attempting apply" << LL_ENDL;
    }
}

void LLFloaterEditExtDayCycle::onInventoryCreated(LLUUID asset_id, LLUUID inventory_id, LLSD results)
{
    LL_WARNS("ENVIRONMENT") << "Inventory item " << inventory_id << " has been created with asset " << asset_id << " results are:" << results << LL_ENDL;

    setFocus(TRUE);                 // Call back the focus...
    loadInventoryItem(inventory_id);
}

void LLFloaterEditExtDayCycle::onInventoryUpdated(LLUUID asset_id, LLUUID inventory_id, LLSD results)
{
    LL_WARNS("ENVIRONMENT") << "Inventory item " << inventory_id << " has been updated with asset " << asset_id << " results are:" << results << LL_ENDL;

    if (inventory_id != mInventoryId)
    {
        loadInventoryItem(inventory_id);
    }
}

void LLFloaterEditExtDayCycle::doImportFromDisk()
{   // Load a a legacy Windlight XML from disk.

    LLFilePicker& picker = LLFilePicker::instance();
    if (picker.getOpenFile(LLFilePicker::FFLOAD_XML))
    {
        std::string filename = picker.getFirstFile();

        LL_WARNS("LAPRAS") << "Selected file: " << filename << LL_ENDL;
        LLSettingsDay::ptr_t legacyday = LLEnvironment::createDayCycleFromLegacyPreset(filename);

        if (!legacyday)
        {   // *TODO* Put up error dialog here.  Could not create water from filename
            return;
        }

        mEditDay = legacyday;
        mCurrentTrack = 1;
        updateSlider();
        updateEditEnvironment();
        syncronizeTabs();
        refresh();
    }
}

bool LLFloaterEditExtDayCycle::canUseInventory() const
{
    return LLEnvironment::instance().isInventoryEnabled();
}

bool LLFloaterEditExtDayCycle::canApplyRegion() const
{
    return gAgent.canManageEstate();
}

bool LLFloaterEditExtDayCycle::canApplyParcel() const
{
    LLParcelSelectionHandle handle(LLViewerParcelMgr::instance().getParcelSelection());
    LLParcel *parcel(nullptr);

    if (handle)
        parcel = handle->getParcel();
    if (!parcel)
        parcel = LLViewerParcelMgr::instance().getAgentParcel();

    if (!parcel)
        return false;

    return parcel->allowModifyBy(gAgent.getID(), gAgent.getGroupID()) &&
        LLEnvironment::instance().isExtendedEnvironmentEnabled();
}

void LLFloaterEditExtDayCycle::startPlay()
{
    mIsPlaying = true;
    mFramesSlider->resetCurSlider();
    mPlayTimer.reset();
    mPlayTimer.start();
    gIdleCallbacks.addFunction(onIdlePlay, this);
    mPlayStartFrame = mTimeSlider->getCurSliderValue();

    getChild<LLView>("play_layout", true)->setVisible(FALSE);
    getChild<LLView>("pause_layout", true)->setVisible(TRUE);
}

void LLFloaterEditExtDayCycle::stopPlay()
{
    if (!mIsPlaying)
        return;

    mIsPlaying = false;
    gIdleCallbacks.deleteFunction(onIdlePlay, this);
    mPlayTimer.stop();
    F32 frame = mTimeSlider->getCurSliderValue();
    selectFrame(frame);

    getChild<LLView>("play_layout", true)->setVisible(TRUE);
    getChild<LLView>("pause_layout", true)->setVisible(FALSE);
}

//static
void LLFloaterEditExtDayCycle::onIdlePlay(void* user_data)
{
    LLFloaterEditExtDayCycle* self = (LLFloaterEditExtDayCycle*)user_data;

    F32 prcnt_played = self->mPlayTimer.getElapsedTimeF32() / DAY_CYCLE_PLAY_TIME_SECONDS;
    F32 new_frame = fmod(self->mPlayStartFrame + prcnt_played, 1.f);

    self->mTimeSlider->setCurSliderValue(new_frame); // will do the rounding
    self->mSkyBlender->setPosition(new_frame);
    self->mWaterBlender->setPosition(new_frame);
    self->syncronizeTabs();

}



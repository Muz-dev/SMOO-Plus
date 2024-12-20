#include "server/inf/InfectionMode.hpp"

#include "al/util.hpp"

#include "game/GameData/GameDataFunction.h"
#include "game/GameData/GameDataHolderAccessor.h"
#include "game/Player/PlayerActorBase.h"
#include "game/Player/PlayerActorHakoniwa.h"
#include "game/Player/PlayerFunction.h"

#include "logger.hpp"

#include "rs/util.hpp"
#include "rs/util/PlayerUtil.h"

#include "sead/heap/seadHeapMgr.h"
#include "sead/math/seadVector.h"

#include "server/Client.hpp"
#include "server/DeltaTime.hpp"
#include "server/gamemode/GameModeManager.hpp"
#include "server/gamemode/GameModeFactory.hpp"
#include "server/inf/InfectionPacket.hpp"

InfectionMode::InfectionMode(const char* name) : GameModeBase(name) {}

void InfectionMode::init(const GameModeInitInfo& info) {
    mSceneObjHolder = info.mSceneObjHolder;
    mMode           = info.mMode;
    mCurScene       = (StageScene*)info.mScene;
    mPuppetHolder   = info.mPuppetHolder;

    GameModeInfoBase* curGameInfo = GameModeManager::instance()->getInfo<GameModeInfoBase>();

    sead::ScopedCurrentHeapSetter heapSetter(GameModeManager::instance()->getHeap());

    if (curGameInfo) {
        Logger::log("Gamemode info found: %s %s\n", GameModeFactory::getModeString(curGameInfo->mMode), GameModeFactory::getModeString(info.mMode));
    } else {
        Logger::log("No gamemode info found\n");
    }

    if (curGameInfo && curGameInfo->mMode == mMode) {
        sead::ScopedCurrentHeapSetter heapSetter(GameModeManager::getSceneHeap());
        mInfo = (InfectionInfo*)curGameInfo;
        mModeTimer = new GameModeTimer(mInfo->mHidingTime);
        Logger::log("Reinitialized timer with time %d:%.2d\n", mInfo->mHidingTime.mMinutes, mInfo->mHidingTime.mSeconds);
    } else {
        if (curGameInfo) {
            delete curGameInfo; // attempt to destory previous info before creating new one
        }

        mInfo = GameModeManager::instance()->createModeInfo<InfectionInfo>();

        mModeTimer = new GameModeTimer();
    }

    sead::ScopedCurrentHeapSetter heapSetterr(GameModeManager::getSceneHeap());

    mModeLayout = new InfectionIcon("InfectionIcon", *info.mLayoutInitInfo);

    mModeLayout->showSeeking();

    mModeTimer->disableTimer();
}

void InfectionMode::processPacket(Packet* _packet) {
    InfectionPacket* packet     = (InfectionPacket*)_packet;
    InFUpdateType      updateType = packet->updateType();

    // if the packet is for our player, edit info for our player
    if (packet->mUserID == Client::getClientId()) {
        if (updateType & InFUpdateType::TIME) {
            mInfo->mHidingTime.mMilliseconds = 0.0;
            mInfo->mHidingTime.mSeconds      = packet->seconds;
            mInfo->mHidingTime.mMinutes      = packet->minutes % 60;
            mInfo->mHidingTime.mHours        = packet->minutes / 60;
            mModeTimer->setTime(mInfo->mHidingTime);
        }

        if (updateType & InFUpdateType::STATE) {
            updateTagState(packet->isIt);
        } else if (updateType & InFUpdateType::TIME) {
            Client::sendGameModeInfPacket();
        }

        return;
    }

    PuppetInfo* other = Client::findPuppetInfo(packet->mUserID, false);
    if (!other) {
        return;
    }

    if (updateType & InFUpdateType::STATE) {
        other->isIt = packet->isIt;
    }

    if (updateType & InFUpdateType::TIME) {
        other->seconds = packet->seconds;
        other->minutes = packet->minutes;
    }
}

Packet* InfectionMode::createPacket() {
    if (!isModeActive()) {
        DisabledGameModeInf* packet = new DisabledGameModeInf(Client::getClientId());
        return packet;
    }

    InfectionPacket* packet = new InfectionPacket();
    packet->mUserID    = Client::getClientId();
    packet->isIt       = isPlayerSeeking();
    packet->seconds    = mInfo->mHidingTime.mSeconds;
    packet->minutes    = mInfo->mHidingTime.mMinutes + mInfo->mHidingTime.mHours * 60;
    packet->setUpdateType(static_cast<InFUpdateType>(InFUpdateType::STATE | InFUpdateType::TIME));
    return packet;
}

void InfectionMode::begin() {
    unpause();

    mIsFirstFrame = true;
    mInvulnTime   = 0.0f;

    GameModeBase::begin();
}

void InfectionMode::end() {
    pause();

    GameModeBase::end();
}

void InfectionMode::pause() {
    GameModeBase::pause();

    mModeLayout->tryEnd();
    mModeTimer->disableTimer();
}

void InfectionMode::unpause() {
    GameModeBase::unpause();

    mModeLayout->appear();

    if (isPlayerSeeking()) {
        mModeTimer->disableTimer();
        mModeLayout->showSeeking();
    } else {
        mModeTimer->enableTimer();
        mModeLayout->showHiding();
    }
}

bool isInInfectAnim = false;

void InfectionMode::update() {

    PlayerActorBase* playerBase = rs::getPlayerActor(mCurScene);

        if(isInInfectAnim && ((PlayerActorHakoniwa*)playerBase)->mPlayerAnimator->isAnimEnd()){
        playerBase->endDemoPuppetable();
        isInInfectAnim = false;
    }

    bool isYukimaru = !playerBase->getPlayerInfo(); // if PlayerInfo is a nullptr, that means we're dealing with the bound bowl racer

    if (mIsFirstFrame) {
        if (mInfo->mIsUseGravityCam && mTicket) {
            al::startCamera(mCurScene, mTicket, -1);
        }
        mIsFirstFrame = false;
    }

    if (rs::isActiveDemoPlayerPuppetable(playerBase)) {
        mInvulnTime = 0.0f; // if player is in a demo, reset invuln time
    }

    if (isPlayerSeeking()) {
        mModeTimer->timerControl();
    } else {
        if (mInvulnTime < 5) {
            mInvulnTime += Time::deltaTime;
        } else if (playerBase) {
            sead::Vector3f offset    = sead::Vector3f(0.0f, 80.0f, 0.0f);
            sead::Vector3f playerPos = al::getTrans(playerBase) + offset;

            if (PlayerFunction::isPlayerDeadStatus(playerBase)) {
                updateTagState(true);
            } else {
                for (size_t i = 0; i < (size_t)mPuppetHolder->getSize(); i++) {
                    PuppetInfo* other = Client::getPuppetInfo(i);
                    if (!other) {
                        Logger::log("Checking %d, hit bounds %d-%d\n", i, mPuppetHolder->getSize(), Client::getMaxPlayerCount());
                        break;
                    }

                    if (!other->isConnected || !other->isInSameStage || other->hnsIsHiding() || isYukimaru) {
                        continue;
                    }

                    if (other->gameMode != mMode && other->gameMode != GameMode::LEGACY) {
                        continue;
                    }

                    float distanceSq = vecDistanceSq(other->playerPos + offset, playerPos); // TODO: remove distance calculations and use hit sensors to determine this

                    if (   distanceSq < 40000.f // non-squared: 200.0
                        && ((PlayerActorHakoniwa*)playerBase)->mDimKeeper->is2DModel == other->is2D
                    ) {
                        playerBase->startDemoPuppetable();
                        al::setVelocityZero(playerBase);
                        rs::faceToCamera(playerBase);
                        ((PlayerActorHakoniwa*)playerBase)->mPlayerAnimator->endSubAnim();
                        ((PlayerActorHakoniwa*)playerBase)->mPlayerAnimator->startAnim("DemoJangoCapSearch");
                        isInInfectAnim = true;

                        updateTagState(true);

                        break;
                    }
                }
            }
        }

        mModeTimer->updateTimer();
    }

    // Gravity
    if (mInfo->mIsUseGravity && !isYukimaru) {
        sead::Vector3f gravity;
        if (rs::calcOnGroundNormalOrGravityDir(&gravity, playerBase, playerBase->getPlayerCollision())) {
            gravity = -gravity;
            al::normalize(&gravity);
            al::setGravity(playerBase, gravity);
            al::setGravity(((PlayerActorHakoniwa*)playerBase)->mHackCap, gravity);
        }

        if (al::isPadHoldL(-1)) {
            if (al::isPadTriggerRight(-1)) {
                if (al::isActiveCamera(mTicket)) {
                    al::endCamera(mCurScene, mTicket, -1, false);
                    mInfo->mIsUseGravityCam = false;
                } else {
                    al::startCamera(mCurScene, mTicket, -1);
                    mInfo->mIsUseGravityCam = true;
                }
            }
        } else if (al::isPadTriggerZL(-1) && al::isPadTriggerLeft(-1)) {
            killMainPlayer(((PlayerActorHakoniwa*)playerBase));
        }
    }

    // Switch roles
    if (al::isPadTriggerUp(-1) && !al::isPadHoldZL(-1)) {
        updateTagState(isPlayerHiding());
    }

    mInfo->mHidingTime = mModeTimer->getTime();
}

bool InfectionMode::showNameTag(PuppetInfo* other) {
    return (
        (other->gameMode != mMode && other->gameMode != GameMode::LEGACY)
        || (isPlayerSeeking() && other->hnsIsSeeking())
    );
}

void InfectionMode::debugMenuControls(sead::TextWriter* gTextWriter) {
    gTextWriter->printf("- L + ← | Enable/disable Infection\n");
    gTextWriter->printf("- [Infection] ↑ | Switch between Runner and Infected\n");
    gTextWriter->printf("- [Infection][Runner] ← | Decrease running time\n");
    gTextWriter->printf("- [Infection]][Runner] → | Increase running time\n");
    gTextWriter->printf("- [Infection][Runner] L + ↓ | Reset running time\n");
    gTextWriter->printf("- [Infection][Gravity] L + → | Toggle gravity camera\n");
}

void InfectionMode::updateTagState(bool isSeeking) {
    mInfo->mIsPlayerIt = isSeeking;

    if (isSeeking) {
        mModeTimer->disableTimer();
        mModeLayout->showSeeking();
    } else {
        mModeTimer->enableTimer();
        mModeLayout->showHiding();
        mInvulnTime = 0;
    }

    Client::sendGameModeInfPacket();
}

void InfectionMode::onBorderPullBackFirstStep(al::LiveActor* actor) {
    if (isUseGravity()) {
        killMainPlayer(actor);
    }
}

void InfectionMode::createCustomCameraTicket(al::CameraDirector* director) {
    mTicket = director->createCameraFromFactory("CameraPoserCustom", nullptr, 0, 5, sead::Matrix34f::ident);
}

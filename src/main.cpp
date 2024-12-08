#include <Geode/Geode.hpp>
#include <Geode/modify/LevelInfoLayer.hpp>
#include <Geode/cocos/support/base64.h>
#include <Geode/cocos/support/zip_support/ZipUtils.h>
#include "FungalShiftArrays.cpp"
#include "Geode/binding/FMODAudioEngine.hpp"
#include "Geode/cocos/label_nodes/CCLabelBMFont.h"
#include "ccTypes.h"

using namespace geode::prelude;

std::string decodeBase64Gzip(const std::string& input) {
    return (std::string) cocos2d::ZipUtils::decompressString(input, false, 0);
}

bool strInArray(std::string key, std::vector<const char*> arr) {
	for (auto item : arr) {
		if (item == key) return true;
	}
	return false;
}

int fungalInputSize  = FUNGAL_INPUTS.size();
int fungalOutputSize = FUNGAL_OUTPUTS.size();


ccColor3B bgColorPrev;

class $modify(HookedLevelInfoLayer, LevelInfoLayer) {

	bool init(GJGameLevel* level, bool challenge) {
		if (!LevelInfoLayer::init(level, challenge)) {
			return false;
		}
		
		auto background = static_cast<CCSprite*>(this->getChildByID("background"));
		bgColorPrev = background->getColor();
		auto winSize = CCDirector::get()->getWinSize();

		auto fungalEcho = CCLabelBMFont::create("test", "bigFont.fnt");
		fungalEcho->setID("fungal-echo");
		fungalEcho->setPosition(winSize.width / 2, 160);
		fungalEcho->setScale(0.7);
		fungalEcho->setOpacity(0);
		fungalEcho->setColor({255, 0, 0});
		fungalEcho->setAlignment(cocos2d::CCTextAlignment::kCCTextAlignmentCenter);
		this->addChild(fungalEcho, 4);

		auto fungalShiftBtn = CCMenuItemSpriteExtra::create(
			CCSprite::create("fungalBtn.png"_spr),
			this,
			menu_selector(HookedLevelInfoLayer::onMyButton)
		);

		auto menu = this->getChildByID("left-side-menu");
		menu->addChild(fungalShiftBtn);

		fungalShiftBtn->setID("fungal-shift-btn"_spr);

		menu->updateLayout();

		return true;
	}

	void fungalShift(std::string& str) {
		int lo = str.find_first_of(',',str.find_first_of(';'))+1;
		int hi = str.find_first_of(',', lo);

		auto fungalInputBlocks = FUNGAL_INPUTS[rand()%fungalInputSize];
		auto fungalOutputBlocks = FUNGAL_OUTPUTS[rand()%fungalOutputSize];
		
		log::debug("{} turned into {}", fungalInputBlocks[0], fungalOutputBlocks[0]);
		auto echoWords = rand()%2 == 0 ? fungalInputBlocks[0] : fungalOutputBlocks[0];
		auto fungalEcho = static_cast<CCLabelBMFont*>(this->getChildByID("fungal-echo"));
		std::string echoMessage = std::format("YOU HEAR THE WORD \"{}\"\nECHOING AND SHIFTING IN COLOURS", echoWords);
		fungalEcho->setString(echoMessage.c_str());

		auto fadeOut = cocos2d::CCFadeOut::create(3);
		fungalEcho->setOpacity(1);
		fungalEcho->runAction(fadeOut);

		auto background = static_cast<CCSprite*>(this->getChildByID("background"));
		auto bgFade = cocos2d::CCTintTo::create(3, bgColorPrev.r, bgColorPrev.g, bgColorPrev.b);
		background->setColor({20, 20, 20});
		background->runAction(bgFade);

		auto fungalOutputBlocksSize = fungalOutputBlocks.size()-1;

		while (lo!=std::string::npos && hi!=std::string::npos) {
			// if (str.substr(lo, hi-lo) == "1") {
			if (strInArray(str.substr(lo, hi-lo),fungalInputBlocks)) {
				auto randomBlock = rand()%fungalOutputBlocksSize+1;
				str.replace(lo,hi-lo,fungalOutputBlocks[randomBlock]);
			}
			lo = str.find_first_of(',', str.find_first_of(';', hi));
			if (lo != std::string::npos) {
				hi = str.find_first_of(',', ++lo);
			}
		}
	}

	void onMyButton(CCObject*) {
		FMODAudioEngine::get()->playEffect("chest08.ogg");
		FMODAudioEngine::get()->playEffect("crystal01.ogg");
		auto fungalString = decodeBase64Gzip(this->m_level->m_levelString);
		fungalShift(fungalString);
		this->m_level->m_levelString = fungalString;
	}
};
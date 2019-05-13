#include "AnimDeformer.h"
#include <string>
#include <cstdio>
#include <fstream>
#include <cstdint>
#include <android/log.h>
#include "type_ptr.hpp"
#include "HalfFloat.h"
#define LOG_TAG "AnimDeformer"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

const std::uint32_t g_nVert = 5832;

bool AnimDeformer::loadExpressionData(const char * expressionFilePath, const char * modifierFilePath, const char * transformFilePath)
{
	// load transform
	std::ifstream transStream{ transformFilePath };
	if (!transStream.good()) {
		LOGE("invalid path for transform: %s\n", transformFilePath);
		return false;
	}
	for (size_t i = 0; i < 9; i++)
		transStream >> glm::value_ptr(m_trans)[i];

	std::string path = expressionFilePath;
	auto parentPath = path.substr(0, path.find_last_of("/\\"));
	auto animPath = parentPath + "/animations.dat";
	std::ifstream animStream{ animPath, std::ios::binary };
	if (!animStream.good()) {
		LOGE("invalid path for animation: %s\n", animPath.c_str());
		return false;
	}

	std::vector<std::uint16_t[3]> hFrame(g_nVert);
	std::uint32_t nAnim = 0;
	animStream.read(reinterpret_cast<char*>(&nAnim), sizeof(nAnim));
	m_anims.resize(nAnim);
	for (auto& anim : m_anims) {
		animStream.read(reinterpret_cast<char*>(&anim.interval), sizeof(anim.interval));
		std::uint32_t nFrame = 0;
		animStream.read(reinterpret_cast<char*>(&nFrame), sizeof(nFrame));
		anim.frames.resize(nFrame);
		for (auto& frame : anim.frames) {
			frame.resize(g_nVert);
			animStream.read(reinterpret_cast<char*>(hFrame.data()), sizeof(hFrame[0])*g_nVert);
			halfp2singles(frame.data(), hFrame.data(), g_nVert * 3);
			for (auto& vert : frame) vert = m_trans * vert;
		}
	}
	return true;
}

void AnimDeformer::clearExpressionData()
{
	LOGI("clearExpressionData");
}

void AnimDeformer::resetDeformer()
{
	LOGI("resetDeformer");
	m_curAnim = 0;
	m_curFrame = 0;
}

void AnimDeformer::runDeform(const vector<OBJVertex> & inVertexes, vector<OBJVertex> & outVertexes)
{
	LOGI("runDeform");
	const auto& anim = m_anims[m_curAnim];
	auto index = m_curFrame / anim.interval;
	if (index >= anim.frames.size()) {
		m_curFrame = 0;
		index = 0;
	}
	auto in = reinterpret_cast<const glm::vec3*>(inVertexes.data());
	auto out = reinterpret_cast<glm::vec3*>(outVertexes.data());
	for (size_t i = 0; i < g_nVert; i++) {
		out[i] = in[i] + anim.frames[index][i];
	}
	m_curFrame++;
}

void AnimDeformer::setExpressionID(int id)
{
	LOGI("setExpressionID");
}

void AnimDeformer::setModifierID(int id)
{
	LOGI("setModifierID");
	if (id <= m_anims.size()) m_curAnim = id;
	else LOGE("id out of range\n");
}

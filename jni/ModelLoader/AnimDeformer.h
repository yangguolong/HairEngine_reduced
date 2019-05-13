#ifndef AnimDeformer_h__
#define AnimDeformer_h__

#include "IHeadDeformer.h"
#include "glm.hpp"
#include <cstdint>

class AnimDeformer : public IHeadDeformer
{
public:
	bool loadExpressionData(const char * expressionFilePath, const char * modifierFilePath, const char * transformFilePath) override;
	void clearExpressionData() override;
	void resetDeformer() override;
	void runDeform(const vector<OBJVertex> & inVertexes, vector<OBJVertex> & outVertexes) override;
	void setExpressionID(int id) override;
	void setModifierID(int id) override;
private:
	// key frame animation
	struct Animation
	{
		std::vector<std::vector<glm::vec3>>	frames;
		std::uint32_t			interval;
	};
	std::vector<Animation>	m_anims;
	glm::mat3				m_trans;

	std::uint32_t			m_curAnim = 0;
	std::uint32_t			m_curFrame = 0;
};

#endif // AnimDeformer_h__

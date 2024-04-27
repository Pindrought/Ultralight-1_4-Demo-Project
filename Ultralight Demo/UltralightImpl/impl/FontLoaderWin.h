#include <PCH.h>

class FontLoaderWin : public ul::FontLoader {
public:
	FontLoaderWin() {}
	virtual ~FontLoaderWin() {}
	virtual ul::String fallback_font() const override;
	virtual ul::String fallback_font_for_characters(const ul::String& characters, int weight, bool italic) const override;
	virtual ul::RefPtr<ul::FontFile> Load(const ul::String& family, int weight, bool italic) override;
protected:
	std::map<uint32_t, ul::RefPtr<ul::Buffer>> fonts_;
};
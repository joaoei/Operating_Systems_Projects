#include <fstream>

namespace bbb {
	class GPIO {
		std::fstream stream;
		std::string path;

	public:
		GPIO(const int &id);

		void setValueOn();
		void setValueOff();
		bool getValue();

		void setModeIn();
		void setModeOut();
		std::string getMode();
	};
}

#include "editor/editor.h"

int main(int argc, const char** argv)
{
	DangoFM::Editor* ed = new DangoFM::Editor();
	if (ed->Init() == 0)
	{
		ed->Run();
	}

	delete ed;
}

#include <xerrori.hpp>




void xterminate(const char* message, int line, const char* file)
{
	if (errno == 0)  fprintf(stderr, "%s\n", message);
	else
	{
		//size_t errmsglen = strerrorlen_s(errno) + 1; // https://stackoverflow.com/questions/44430141/missing-c11-strerrorlen-s-function-under-msvc-2017
		char errmsg[256];
		strerror_s(errmsg, 256, errno);
		fprintf(stderr, "%s: %s\n", message, errmsg);
	}
	fprintf(stderr, "Line: %d, File: %s\n", line, file);

	exit(1);
}

void xassert(bool expression, const char* message, int line, const char* file)
{
	if (expression == false) xterminate(message, line, file);
}


// funzioni GLFW
#ifdef _glfw3_h_

int xglfwInit(int line, const char* file)
{
	if (!glfwInit())
	{
		std::cout << "glfw initialization creation failed. Line: " << line << " File: " << file << std::endl;
		exit(1);
	}
}

GLFWwindow* xglfwCreateWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share, int linea, const char* file)
{
	GLFWwindow* window = glfwCreateWindow(width, height, title, monitor, share);
	if (!window)
	{
		glfwTerminate();
		std::cout << "glfw window creation failed. Line: " << linea << " File: " << file << std::endl;
		exit(1);
	}
	return window;
}

int xglfwGetKey(GLFWwindow* window, int key, int line, const char* file)
{
	const char* description;

	int r = glfwGetKey(window, key);
	if (int e = glfwGetError(&description))
	{
		std::cout << "Line: " << line << " File: " << file << " Error: " << e << std::endl << "Description: " << description << std::endl;
		glfwTerminate();
		exit(1);
	}
	return r;
}

#endif
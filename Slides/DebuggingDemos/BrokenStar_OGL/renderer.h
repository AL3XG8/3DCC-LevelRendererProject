
void PrintLabeledDebugString(const char* label, const char* toPrint)
{
	std::cout << label << toPrint << std::endl;
#if defined WIN32 //OutputDebugStringA is a windows-only function 
	OutputDebugStringA(label);
	OutputDebugStringA(toPrint);
#endif
}

#ifndef NDEBUG
// Used to print debug infomation from OpenGL, pulled straight from the official OpenGL wiki.
void APIENTRY
MessageCallback(GLenum source, GLenum type, GLuint id,
	GLenum severity, GLsizei length,
	const GLchar* message, const void* userParam) {


	std::string errMessage;
	errMessage = (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : "");
	errMessage += " type = ";
	errMessage += type;
	errMessage += ", severity = ";
	errMessage += severity;
	errMessage += ", message = ";
	errMessage += message;
	errMessage += "\n";

	PrintLabeledDebugString("GL CALLBACK: ", errMessage.c_str());
}
#endif

// Creation, Rendering & Cleanup
class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GOpenGLSurface ogl;
	// what we need at a minimum to draw a triangle
	GLuint vertexArray = 0;
	GLuint vertexBufferObject = 0;
	GLuint vertexShader = 0;
	GLuint fragmentShader = 0;
	GLuint shaderExecutable = 0;

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GOpenGLSurface _ogl)
	{
		win = _win;
		ogl = _ogl;
		InitializeGraphics();
	}
private:
	void InitializeGraphics()
	{

#ifndef NDEBUG
		BindDebugCallback();
#endif
		// Raw array of floats is very hard to read, ideally we would use a vertex structure here instead.
		InitializeVertexBuffer();

		CompileVertexShader();
		CompileFragmentShader();
		CreateExecutableShaderProgram();
	}

#ifndef NDEBUG
	void BindDebugCallback()
	{
		// In debug mode we link openGL errors to the console
		glEnable(GL_DEBUG_OUTPUT);
		glDebugMessageCallback(MessageCallback, 0);
	}
#endif

	void InitializeVertexBuffer()
	{
		std::vector<float> verts;
		BuildGoldenStar(verts);
		CreateVertexBuffer(&verts, sizeof(verts));
	}

	void BuildGoldenStar(std::vector<float>& verts)
	{
		// Build a golden star (to keep things simple you can assume this loop is correct, debug everything else though!)
		for (int i = 1, j = 0; i <= 360; i += 36, ++j) {
			verts.push_back(sinf(G_DEGREE_TO_RADIAN(i)) * ((j % 2) ? 1.0f : 0.5f)); // A x
			verts.push_back(-cosf(G_DEGREE_TO_RADIAN(i)) * ((j % 2) ? 1.0f : 0.5f)); // A y
			verts.push_back(fabs(cosf(G_DEGREE_TO_RADIAN(i)))); // A r
			verts.push_back(fabs(cosf(G_DEGREE_TO_RADIAN(i)))); // A g
			verts.push_back(0); // A b

			verts.push_back(sinf(G_DEGREE_TO_RADIAN(i + 36)) * (((j + 1) % 2) ? 1.0f : 0.5f)); // B x
			verts.push_back(-cosf(G_DEGREE_TO_RADIAN(i + 36)) * (((j + 1) % 2) ? 1.0f : 0.5f)); // B y
			verts.push_back(fabs(cosf(G_DEGREE_TO_RADIAN(i)))); // B r
			verts.push_back(fabs(cosf(G_DEGREE_TO_RADIAN(i)))); // B g
			verts.push_back(0); // B b

			verts.push_back(0); // C x
			verts.push_back(0); // C y
			verts.push_back(1); // C r
			verts.push_back(1); // C g
			verts.push_back(1); // C b
		}
	}

	void CreateVertexBuffer(void* data, unsigned int sizeInBytes)
	{
		glGenVertexArrays(1, &vertexArray);
		glGenBuffers(1, &vertexBufferObject);
		glBindVertexArray(vertexArray);
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, sizeInBytes, data, GL_STATIC_DRAW);
	}

	void CompileVertexShader()
	{
		vertexShader = glCreateShader(GL_VERTEX_SHADER);

		std::string vertexShaderSource = ReadFileIntoString("../Shaders/VertexShader.glsl");
		const GLchar* strings[1] = { vertexShaderSource.c_str() };
		const GLint lengths[1] = { vertexShaderSource.length() };
		glShaderSource(vertexShader, 1, strings, lengths);
		glCompileShader(vertexShader);

		char errors[1024]; GLint result;
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &result);
		if (result == false)
		{
			glGetShaderInfoLog(vertexShader, 1024, NULL, errors);
			PrintLabeledDebugString("Vertex Shader Errors:\n", errors);
			abort();
			return;
		}
	}

	void CompileFragmentShader()
	{
		char errors[1024]; GLint result;
		fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		std::string fragmentShaderSource = ReadFileIntoString("../Shaders/VertexShader.glsl");
		const GLchar* strings[1] = { fragmentShaderSource.c_str() };
		const GLint lengths[1] = { fragmentShaderSource.length() };
		glShaderSource(fragmentShader, 1, strings, lengths);

		glCompileShader(fragmentShader);
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &result);
		if (result == false)
		{
			glGetShaderInfoLog(fragmentShader, 1024, NULL, errors);
			PrintLabeledDebugString("Fragment Shader Errors:\n", errors);
			abort();
			return;
		}
	}

	void CreateExecutableShaderProgram()
	{
		char errors[1024]; GLint result;
		shaderExecutable = glCreateProgram();
		glAttachShader(shaderExecutable, vertexShader);
		glAttachShader(shaderExecutable, fragmentShader);
		glLinkProgram(shaderExecutable);
		glGetProgramiv(shaderExecutable, GL_LINK_STATUS, &result);
		if (result == false)
		{
			glGetProgramInfoLog(shaderExecutable, 1024, NULL, errors);
			std::cout << errors << std::endl;
		}
	}





public:
	void Render()
	{
		SetUpPipeline();

		glDrawArrays(GL_TRIANGLES, 0, 27);
	}

	void SetUpPipeline()
	{
		SetVertexAttributes();
		glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
		glUseProgram(shaderExecutable);
		glBindVertexArray(vertexArray);
	}

	void SetVertexAttributes()
	{
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(-1);
	}


public:
	~Renderer()
	{
		// free resources
		glDeleteVertexArrays(1, &vertexArray);
		glDeleteBuffers(1, &vertexBufferObject);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		glDeleteProgram(shaderExecutable);
	}

};

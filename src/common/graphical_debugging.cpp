#include <graphical_debugging.hpp>

GraphicalDebugObject::GraphicalDebugObject(Shapes shape_type)
{
    switch (shape_type)
    {
    case CUBE:
        shape = shape_maker::cube();
        break;
    case FRAME:
        shape = shape_maker::frame();
        break;
    case LINE:
        shape = shape_maker::line();
        break;
    case CYLINDER:
        shape = shape_maker::cylinder(10);
        break;
    case QUAD:
        shape = shape_maker::quad();
        break;
    case RECTANGLE:
        shape = shape_maker::rectangle(2, 1);
        break;
    case TORUS:
        xterminate("Graphical debugging shape non implementata.", QUI);
        break;
    case PYRAMID:
        shape = shape_maker::pyramid();
        break;
    case ICOSAHEDRON:
        xterminate("Graphical debugging shape non implementata.", QUI);
        break;
    case SPHERE:
        shape = shape_maker::sphere(10);
        break;
    case CONE:
        shape = shape_maker::cone(2, 4, 10);
        break;
    default:
        xterminate("Shape inesistente", QUI);
        break;
    }
}

renderable GraphicalDebugObject::GetRenderable() { return shape; }

void GraphicalDebugObject::Draw(matrix_stack& stack, shader& _shader, float scale)
{
    glUseProgram(_shader.program);

    shape.bind();
    stack.push();
    stack.mult(shape.transform);
    if (scale != 1.f) stack.mult(glm::scale(glm::mat4(1), glm::vec3(scale)));

    glUniformMatrix4fv(_shader["uModel"], 1, GL_FALSE, &stack.m()[0][0]);

    glDrawElements(shape().mode, shape().count, shape().itype, 0);
    stack.pop();
}

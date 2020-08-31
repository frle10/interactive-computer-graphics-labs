#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace std;

int main(int argc, char** argv)
{
    cout << "1st laboratory exercise, version A\n\n";
    cout << "2nd task\n";
    cout << "----------------------\n\n";

    cout << "Input equation parameters:\n";

    double array[12] = {0};
    for (int i = 0; i < 12; i++)
    {
        cin >> array[i];
    }

    glm::mat4x3 fullSystemMatrix = glm::transpose(glm::make_mat3x4(array));

    glm::mat3 matrixA = glm::mat3(fullSystemMatrix);

    glm::vec3 rightSide = glm::column(fullSystemMatrix, 3);

    glm::vec3 solution = glm::inverse(matrixA) * rightSide;
    cout << "\nThe solution vector is: " << to_string(solution) << "\n\n";

    return 0;
}

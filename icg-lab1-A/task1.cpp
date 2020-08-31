#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace std;

int main(int argc, char** argv)
{
    cout << "1st laboratory exercise, version A\n\n";
    cout << "1st task\n";
    cout << "----------------------\n\n";

    glm::vec3 v1 = glm::vec3(2, 3, -4) + glm::vec3(-1, 4, -1);
    cout << "v1 = " << glm::to_string(v1) << "\n\n";

    double scalarProduct = glm::dot(v1, glm::vec3(-1, 4, -1));
    cout << "s = " << scalarProduct << "\n\n";

    glm::vec3 crossProduct = glm::cross(v1, glm::vec3(2, 2, 4));
    cout << "v2 = " << to_string(crossProduct) << "\n\n";

    glm::vec3 normalizedVector = glm::normalize(crossProduct);
    cout << "v3 = " << to_string(normalizedVector) << "\n\n";

    glm::vec3 oppositeVector = -crossProduct;
    cout << "v4 = " << to_string(oppositeVector) << "\n\n";

    double arrayA[9] = {1, 2, 4, 2, 1, 5, 3, 3, 1};
    double arrayB[9] = {-1, 5, -4, 2, -2, -1, -3, 7, 3};
    double arrayC[16] = {1, 0, 0, 2, 0, 2, 0, 3, 0, 0, 1, 3, 0, 0, 0, 1};

    glm::mat3 matrixA = glm::make_mat3(arrayA);
    glm::mat3 matrixB = glm::make_mat3(arrayB);
    glm::mat4 matrixC = glm::make_mat4(arrayC);

    glm::mat3 addNoTransposition = matrixA + matrixB;
    glm::mat3 mulTransposition = matrixA * glm::transpose(matrixB);
    glm::mat3 mulInverse = matrixA * glm::inverse(matrixB);
    glm::vec4 vec4MulMat4 = glm::vec4(1, 2, 3, 1) * matrixC;

    cout << "M1 = " << to_string(addNoTransposition) << "\n\n";
    cout << "M2 = " << to_string(mulTransposition) << "\n\n";
    cout << "M3 = " << to_string(mulInverse) << "\n\n";
    cout << "V = " << to_string(vec4MulMat4) << "\n\n";

    return 0;
}

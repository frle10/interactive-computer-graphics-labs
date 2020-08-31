#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace std;

int main(int argc, char** argv)
{
    cout << "1st laboratory exercise, version A\n\n";
    cout << "3rd task\n";
    cout << "----------------------\n\n";

    cout << "Input coordinates for triangle vertices and an additional point in 3D space:\n\n";
    cout << "A: ";

    double coordA[3] = {0};
    for(int i = 0; i < 3; i++)
    {
        cin >> coordA[i];
    }

    cout << "\nB: ";
    double coordB[3] = {0};
    for(int i = 0; i < 3; i++)
    {
        cin >> coordB[i];
    }

    cout << "\nC: ";
    double coordC[3] = {0};
    for(int i = 0; i < 3; i++)
    {
        cin >> coordC[i];
    }

    cout << "\nT: ";
    double coordT[3] = {0};
    for(int i = 0; i < 3; i++)
    {
        cin >> coordT[i];
    }

    glm::vec3 vectorA = glm::make_vec3(coordA);
    glm::vec3 vectorB = glm::make_vec3(coordB);
    glm::vec3 vectorC = glm::make_vec3(coordC);
    glm::vec3 vectorT = glm::make_vec3(coordT);

    glm::mat3 triangleMatrix = glm::mat3(vectorA, vectorB, vectorC);

    glm::vec3 baricentricCoordinates = glm::inverse(triangleMatrix) * vectorT;
    cout << "\nThe solution vector is: " << to_string(baricentricCoordinates) << "\n\n";

    return 0;
}

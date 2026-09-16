#pragma once

#include "components/simple_scene.h"
#include "lab_m1/lab5/lab_camera.h"
#include "components/text_renderer.h"

#include <vector>
#include <string>
#include <deque>

namespace m1
{
    enum RailType {
        RAIL_NORMAL = 0,
        RAIL_BRIDGE = 1,
        RAIL_TUNNEL = 2
    };

    enum DesiredDir {
        DIR_STRAIGHT = 0, 
        DIR_LEFT = 1,      
        DIR_RIGHT = 2,      
        DIR_BACK = 3        
    };

    struct Rail {
        glm::vec3 startPosition;
        glm::vec3 endPosition;
        std::vector<Rail*> neighbors;
        RailType type;

        Rail() : type(RAIL_NORMAL) {}
    };

    struct TrainComponent {
        Rail* currentRail = nullptr;
        float progress = 0.f;
        float speed = 10.f;
        glm::vec3 position = glm::vec3(0);
        float rotation = 0.f;
        bool forward = true;
    };

    struct Station {
        glm::vec3 position = glm::vec3(0);
        std::string meshName;
        int resourceType = 0;

        bool resourceAvailable = true;
        float respawnTimer = 0.f;
    };

    struct AABB {
        glm::vec3 min = glm::vec3(0);
        glm::vec3 max = glm::vec3(0);
    };

    class Tema2 : public gfxc::SimpleScene
    {
    public:
        Tema2();
        ~Tema2();

        void Init() override;

    private:
        void RenderGameOverUI();
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        Mesh* CreateMesh(const char* name, const std::vector<VertexFormat>& vertices, const std::vector<unsigned int>& indices);
        void CreateCube(const char* name, glm::vec3 color);
        void CreateCylinder(const char* name, glm::vec3 color);
        void CreateSphere(const char* name, glm::vec3 color);
        void CreateCone(const char* name, glm::vec3 color);

        void RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix);
        void RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, float timeFactor);

        void RenderTerrain();
        void RenderRails();
        void RenderStations(float timeFactor);
        void RenderOrderUI();
        void RenderTrainAssembly(const TrainComponent& t, bool isLocomotive);

        void BuildGraphTrack();
        void InitStations();
        void GenerateOrder();

        Rail* AddRail(const glm::vec3& a, const glm::vec3& b, RailType type);
        void Connect(Rail* from, Rail* to);

        Rail* ChooseNextRail(const TrainComponent& t, DesiredDir desired);
        void UpdateTrainMovement(TrainComponent& t, float deltaTime);

        void UpdateStationsRespawn(float dt);
        void HandleStationInteractions();

        static bool IntersectAABB(const AABB& a, const AABB& b);
        static AABB MakeAABB(const glm::vec3& center, const glm::vec3& halfExtents);
        AABB GetLocomotiveAABB() const;
        AABB GetStationAABB(const Station& s) const;
        AABB GetResourceAABB(const Station& s) const;

        void RenderMinimap(float timeFactor);

        void DetectIntersections();
        void RenderIntersectionPoles();

        void RenderTimerUI();

    protected:
        float gameOverStartTime = -1.0f;   
        float gameOverDuration = 10.0f;    
        gfxc::TextRenderer* textRenderer = nullptr;
        implemented::Camera* camera = nullptr;
        glm::mat4 projectionMatrix;

        implemented::Camera* minimapCamera = nullptr;
        glm::mat4 orthoMatrix;

        std::vector<Rail*> track;
        std::vector<Station> stations;

        TrainComponent locomotive;
        std::vector<TrainComponent> wagons;

        std::deque<glm::vec3> trainHistoryPos;
        std::deque<float> trainHistoryRot;

        float maxTime = 60.0f;
        float timeRemaining = 60.0f;
        bool gameOver = false;

        std::deque<int> activeOrder;

        int lastStationIdx = -1;

        DesiredDir desiredDir = DIR_STRAIGHT;

        int score = 0;

        float trainScale = 1.8f;

        int numWagons = 3;              
        int wagonDelayFrames = 70;

        std::vector<glm::vec3> intersections;
    };
} // namespace m1

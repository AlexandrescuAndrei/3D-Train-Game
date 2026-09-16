#include "lab_m1/Tema2/Tema2.h"
#include "components/text_renderer.h"

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <algorithm>

#include <glm/gtc/type_ptr.hpp>

namespace m1
{
    static const glm::vec3 C_VERDE = glm::vec3(0.0f, 1.0f, 0.0f);
    static const glm::vec3 C_ALBASTRU = glm::vec3(0.0f, 0.0f, 1.0f);
    static const glm::vec3 C_GALBEN = glm::vec3(1.0f, 1.0f, 0.0f);
    static const glm::vec3 C_ROSU = glm::vec3(1.0f, 0.0f, 0.0f);
    static const glm::vec3 C_ROZ = glm::vec3(1.0f, 0.0f, 1.0f);
    static const glm::vec3 C_NEGRU = glm::vec3(0.06f, 0.06f, 0.08f);

    static const glm::vec3 C_IARBA = glm::vec3(0.35f, 0.82f, 0.35f);
    static const glm::vec3 C_APA = glm::vec3(0.2f, 0.6f, 1.0f);
    static const glm::vec3 C_MUNTE = glm::vec3(0.55f, 0.4f, 0.2f);

    static const glm::vec3 C_LEMN = glm::vec3(0.6f, 0.4f, 0.2f);
    static const glm::vec3 C_LEMN_INC = glm::vec3(0.4f, 0.25f, 0.1f);
    static const glm::vec3 C_BETON = glm::vec3(0.55f, 0.55f, 0.6f);
    static const glm::vec3 C_BETON_INC = glm::vec3(0.3f, 0.3f, 0.35f);

    Tema2::Tema2() {}

    Tema2::~Tema2()
    {
        for (auto r : track) delete r;
        track.clear();

        delete camera;
        delete minimapCamera;
		delete textRenderer;
    }

    void Tema2::Init()
    {
        glEnable(GL_DEPTH_TEST);

        camera = new implemented::Camera();

        camera->Set(
            glm::vec3(0, 95, 95),
            glm::vec3(0, 0, 0),
            glm::vec3(0, 1, 0)
        );

        projectionMatrix = glm::perspective(
            RADIANS(70),
            window->props.aspectRatio,
            0.01f,
            600.0f
        );

        minimapCamera = new implemented::Camera();
        minimapCamera->Set(glm::vec3(0, 120, 0), glm::vec3(0, 0, 0), glm::vec3(0, 0, -1));
        orthoMatrix = glm::ortho(-120.0f, 120.0f, -120.0f, 120.0f, 0.1f, 500.0f);

        Shader* stationShader = new Shader("Station");
        stationShader->AddShader("src/lab_m1/Tema2/shaders/VertexShader.glsl", GL_VERTEX_SHADER);
        stationShader->AddShader("src/lab_m1/Tema2/shaders/FragmentShader.glsl", GL_FRAGMENT_SHADER);
        stationShader->CreateAndLink();
        shaders[stationShader->GetName()] = stationShader;

        CreateCube("box_yellow", C_GALBEN);
        CreateCube("box_green", C_VERDE);
        CreateCube("box_black", C_NEGRU);

        CreateCube("box_grass", C_IARBA);
        CreateCube("box_water", C_APA);
        CreateCube("box_mountain", C_MUNTE);

        CreateCube("box_wood", C_LEMN);
        CreateCube("box_wood_dark", C_LEMN_INC);
        CreateCube("box_concrete", C_BETON);
        CreateCube("box_concrete_dark", C_BETON_INC);

        CreateCylinder("cylinder_blue", C_ALBASTRU);
        CreateCylinder("cylinder_pink", C_ROZ);
        CreateCylinder("wheel_red", C_ROSU);

        CreateCube("station_main_base", glm::vec3(0.25f, 0.75f, 0.25f));
        CreateCube("station_platform", glm::vec3(0.35f, 0.35f, 0.35f));
        CreateCube("station_house", glm::vec3(0.7f, 0.7f, 0.75f));

        CreateSphere("resource_sphere", glm::vec3(1.0f, 0.5f, 0.0f));
        CreateCone("resource_cone", glm::vec3(0.5f, 0.0f, 0.5f));
        CreateCylinder("resource_cylinder", glm::vec3(0.0f, 1.0f, 1.0f));

        CreateCube("box_white", glm::vec3(0.95f, 0.95f, 0.95f));
        CreateCube("box_red", glm::vec3(0.90f, 0.10f, 0.10f));

        BuildGraphTrack();
        InitStations();

        if (!track.empty()) {
            locomotive.currentRail = track[0];
            locomotive.progress = 0.f;
            locomotive.speed = 20.0f;
            locomotive.position = track[0]->startPosition;
        }

        trainHistoryPos.clear();
        trainHistoryRot.clear();

        wagons.clear();
        wagons.resize(numWagons);
        for (int i = 0; i < numWagons; i++) {
            wagons[i] = locomotive;
        }

        maxTime = 60.f;
        timeRemaining = maxTime;
        gameOver = false;
        score = 0;

        GenerateOrder();

        glm::ivec2 res = window->GetResolution();

        textRenderer = new gfxc::TextRenderer(window->props.selfDir, (GLuint)res.x, (GLuint)res.y);

        textRenderer->Load("assets/fonts/Hack-Bold.ttf", 36);
    }

    void Tema2::GenerateOrder()
    {
        activeOrder.clear();
        for (int i = 0; i < 5; i++) {
            int r = (std::rand() % 3) + 1;
            activeOrder.push_back(r);
        }

        timeRemaining = maxTime;
        gameOver = false;
    }


    Rail* Tema2::AddRail(const glm::vec3& a, const glm::vec3& b, RailType type)
    {
        Rail* r = new Rail();
        r->startPosition = a;
        r->endPosition = b;
        r->type = type;
        track.push_back(r);
        return r;
    }

    void Tema2::Connect(Rail* from, Rail* to)
    {
        if (!from || !to) return;
        from->neighbors.push_back(to);
    }

    void Tema2::BuildGraphTrack()
    {
        const float L = 60.f;
        const float H = 40.f;

        glm::vec3 A(-L, 0, -H);
        glm::vec3 B(L, 0, -H);
        glm::vec3 C(L, 0, H);
        glm::vec3 D(-L, 0, H);

        glm::vec3 M1(0, 0, -H);
        glm::vec3 M2(L, 0, 0);
        glm::vec3 M3(0, 0, H);
        glm::vec3 M4(-L, 0, 0);

        glm::vec3 O(0, 0, 0);

        // loop principal sens orar
        Rail* rA1 = AddRail(A, M1, RAIL_NORMAL);
        Rail* rA2 = AddRail(M1, B, RAIL_NORMAL);

        Rail* rB1 = AddRail(B, M2, RAIL_BRIDGE);
        Rail* rB2 = AddRail(M2, C, RAIL_BRIDGE);

        Rail* rC1 = AddRail(C, M3, RAIL_NORMAL);
        Rail* rC2 = AddRail(M3, D, RAIL_NORMAL);

        Rail* rD1 = AddRail(D, M4, RAIL_TUNNEL);
        Rail* rD2 = AddRail(M4, A, RAIL_TUNNEL);

        // conexiuni loop
        Connect(rA1, rA2);
        Connect(rA2, rB1);
        Connect(rB1, rB2);
        Connect(rB2, rC1);
        Connect(rC1, rC2);
        Connect(rC2, rD1);
        Connect(rD1, rD2);
        Connect(rD2, rA1);


        // intrari spre centru (intersectii)
        Rail* rM1O = AddRail(M1, O, RAIL_NORMAL);
        Rail* rM2O = AddRail(M2, O, RAIL_NORMAL);
        Rail* rM3O = AddRail(M3, O, RAIL_NORMAL);
        Rail* rM4O = AddRail(M4, O, RAIL_NORMAL);

   
        Connect(rA1, rM1O);

        Connect(rB1, rM2O);
        
        Connect(rC1, rM3O);

        Connect(rD1, rM4O);

        Connect(rB1, rB2);

        // iesiri din O (4 directii)
        Rail* rOM1 = AddRail(O, M1, RAIL_NORMAL);
        Rail* rOM2 = AddRail(O, M2, RAIL_NORMAL);
        Rail* rOM3 = AddRail(O, M3, RAIL_NORMAL);
        Rail* rOM4 = AddRail(O, M4, RAIL_NORMAL);

        // din M1
        Connect(rM1O, rA2);
        Connect(rM1O, rOM2);
        Connect(rM1O, rOM4);

        // din M2
        Connect(rM2O, rB2);
        Connect(rM2O, rOM1);
        Connect(rM2O, rOM3);

        // din M3
        Connect(rM3O, rC2);
        Connect(rM3O, rOM2);
        Connect(rM3O, rOM4);

        // din M4
        Connect(rM4O, rD2);
        Connect(rM4O, rOM1);
        Connect(rM4O, rOM3);


        Connect(rOM1, rA2);
        Connect(rOM2, rB2);
        Connect(rOM3, rC2);
        Connect(rOM4, rD2);

        DetectIntersections();
    }


    void Tema2::InitStations()
    {
        stations.clear();

        const float L = 60.f;
        const float H = 40.f;

        const float OUT = 12.0f;

        Station main;
        main.position = glm::vec3(-10.f, 0.f, -10.f);
        main.meshName = "station_main_base";
        main.resourceType = 0;
        stations.push_back(main);

        Station s1;
        s1.position = glm::vec3(-L - OUT, 0.f, 12.f);
        s1.meshName = "station_house";
        s1.resourceType = 1;
        stations.push_back(s1);

        Station s2;
        s2.position = glm::vec3(L + OUT, 0.f, -12.f);
        s2.meshName = "station_house";
        s2.resourceType = 2;
        stations.push_back(s2);

        Station s3;
        s3.position = glm::vec3(10.f, 0.f, -H - OUT);
        s3.meshName = "station_house";
        s3.resourceType = 3;
        stations.push_back(s3);
    }


    void Tema2::RenderSimpleMesh(Mesh* mesh, Shader* shader, const glm::mat4& modelMatrix, float timeFactor)
    {
        if (!mesh || !shader || !shader->GetProgramID()) return;

        shader->Use();
        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));

        GLint locTime = glGetUniformLocation(shader->program, "time_factor");
        glUniform1f(locTime, timeFactor);

        mesh->Render();
    }

    void Tema2::RenderMesh(Mesh* mesh, Shader* shader, const glm::mat4& m)
    {
        if (!mesh || !shader || !shader->GetProgramID()) return;
        shader->Use();
        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projectionMatrix));
        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(m));
        mesh->Render();
    }

    void Tema2::RenderTerrain()
    {
        glm::mat4 grass = glm::translate(glm::mat4(1), glm::vec3(0, -0.60f, 0));
        grass = glm::scale(grass, glm::vec3(320.0f, 0.10f, 320.0f));
        RenderMesh(meshes["box_grass"], shaders["VertexColor"], grass);

        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(-2.0f, -2.0f);

        const float L = 60.f;
        const float H = 40.f;

        const float y = -0.545f;

        {
            glm::mat4 mountain = glm::translate(glm::mat4(1), glm::vec3(-L, y, 0.0f));
            mountain = glm::scale(mountain, glm::vec3(30.0f, 0.02f, 150.0f));
            RenderMesh(meshes["box_mountain"], shaders["VertexColor"], mountain);
        }

        {
            glm::mat4 water = glm::translate(glm::mat4(1), glm::vec3(+L, y, 0.0f));
            water = glm::scale(water, glm::vec3(30.0f, 0.02f, 150.0f));
            RenderMesh(meshes["box_water"], shaders["VertexColor"], water);
        }

        glDisable(GL_POLYGON_OFFSET_FILL);
    }


    void Tema2::RenderRails()
    {
        const float RAIL_OFFSET = 2.5f;
        const float RAIL_WIDTH = 0.5f;
        const float RAIL_HEIGHT = 0.1f;
        for (auto r : track) {
            glm::vec3 center = (r->startPosition + r->endPosition) * 0.5f;
            float len = glm::distance(r->startPosition, r->endPosition);
            if (len < 0.001f) continue;

            glm::vec3 dir = glm::normalize(r->endPosition - r->startPosition);
            float angle = std::atan2(dir.x, dir.z);

            glm::mat4 base = glm::translate(glm::mat4(1), center);
            base = glm::rotate(base, angle, glm::vec3(0, 1, 0));

            if (r->type == RAIL_NORMAL) {
                glm::mat4 mTrack = glm::translate(base, glm::vec3(0, 0.03f, 0));
                mTrack = glm::scale(mTrack, glm::vec3(1.2f, 0.06f, len));
                RenderMesh(meshes["box_black"], shaders["VertexColor"], mTrack);
            }
            else if (r->type == RAIL_BRIDGE) {
                glm::mat4 mWater = glm::translate(base, glm::vec3(0, -0.08f, 0));
                mWater = glm::scale(mWater, glm::vec3(2.6f, 0.05f, len));
                RenderMesh(meshes["box_water"], shaders["VertexColor"], mWater);

                {
                    glm::mat4 railL = glm::translate(base, glm::vec3(-RAIL_OFFSET, 0.08f, 0));
                    railL = glm::scale(railL, glm::vec3(RAIL_WIDTH, RAIL_HEIGHT, len));
                    RenderMesh(meshes["box_black"], shaders["VertexColor"], railL);
                }

                {
                    glm::mat4 railR = glm::translate(base, glm::vec3(+RAIL_OFFSET, 0.08f, 0));
                    railR = glm::scale(railR, glm::vec3(RAIL_WIDTH, RAIL_HEIGHT, len));
                    RenderMesh(meshes["box_black"], shaders["VertexColor"], railR);
                }

                for (int k = 0; k < 6; k++) {
                    float x = -1.6f + k * 0.8f;
                    glm::mat4 strip = glm::translate(base, glm::vec3(x, 0.08f, 0));
                    strip = glm::scale(strip, glm::vec3(0.18f, 0.3f, len));
                    RenderMesh(meshes["box_black"], shaders["VertexColor"], strip);
                }
            }
            else if (r->type == RAIL_TUNNEL) {
                glm::mat4 ground = glm::translate(base, glm::vec3(0, -0.08f, 0));
                ground = glm::scale(ground, glm::vec3(2.8f, 0.05f, len));
                RenderMesh(meshes["box_mountain"], shaders["VertexColor"], ground);

                {
                    glm::mat4 railL = glm::translate(base, glm::vec3(-RAIL_OFFSET, 0.08f, 0));
                    railL = glm::scale(railL, glm::vec3(RAIL_WIDTH, RAIL_HEIGHT, len));
                    RenderMesh(meshes["box_black"], shaders["VertexColor"], railL);
                }

                {
                    glm::mat4 railR = glm::translate(base, glm::vec3(+RAIL_OFFSET, 0.08f, 0));
                    railR = glm::scale(railR, glm::vec3(RAIL_WIDTH, RAIL_HEIGHT, len));
                    RenderMesh(meshes["box_black"], shaders["VertexColor"], railR);
                }

                int numBars = std::max(1, (int)(len / 3.0f));
                for (int i = 0; i < numBars; i++) {
                    float z = -len / 2.0f + (i + 0.5f) * (len / numBars);
                    glm::mat4 bar = glm::translate(base, glm::vec3(0, 0.10f, z));
                    bar = glm::scale(bar, glm::vec3(4.5f, 0.08f, 0.6f));
                    RenderMesh(meshes["box_black"], shaders["VertexColor"], bar);
                }
            }
        }
    }

    void Tema2::RenderStations(float timeFactor)
    {
        const float STATION_SCALE = 1.6f;

        float t = (float)glfwGetTime();
        for (const auto& s : stations) {
            glm::mat4 platform = glm::translate(
                glm::mat4(1),
                s.position + glm::vec3(0, 0.2f * STATION_SCALE, 0)
            );
            platform = glm::scale(
                platform,
                glm::vec3(8.0f, 0.4f, 6.0f) * STATION_SCALE
            );
            RenderMesh(meshes["station_platform"], shaders["VertexColor"], platform);

            glm::mat4 body = glm::translate(
                glm::mat4(1),
                s.position + glm::vec3(0, 1.6f * STATION_SCALE, 0)
            );
            body = glm::scale(
                body,
				glm::vec3(3.5f, 3.0f, 3.0f) * STATION_SCALE * 1.5f
            );

            if (s.resourceType == 0) {
                RenderSimpleMesh(meshes["station_main_base"], shaders["Station"], body, timeFactor);
            }
            else {
                RenderMesh(meshes["station_house"], shaders["VertexColor"], body);
            }

            if (s.resourceType != 0 && s.resourceAvailable) {
                float bob = 0.5f + 0.35f * std::sin(t * 2.3f + (float)s.resourceType);

                glm::vec3 pos = s.position + glm::vec3(0, (5.5f + bob) * STATION_SCALE, 0);

                glm::mat4 m = glm::translate(glm::mat4(1), pos);
                m = glm::rotate(m, t * 1.6f, glm::vec3(0, 1, 0));
                m = glm::scale(m, glm::vec3(2.1f * STATION_SCALE));

                if (s.resourceType == 1) RenderMesh(meshes["resource_sphere"], shaders["VertexColor"], m);
                if (s.resourceType == 2) RenderMesh(meshes["resource_cone"], shaders["VertexColor"], m);
                if (s.resourceType == 3) RenderMesh(meshes["resource_cylinder"], shaders["VertexColor"], m);
            }
        }
    }


    void Tema2::RenderOrderUI()
    {
        const float ORDER_SCALE = 2.4f;
        const float SPACING = 2.4f * ORDER_SCALE;

        glm::vec3 base = stations[0].position
            + glm::vec3(-20.0f, 15.0f, 0.0f);

        float t = (float)glfwGetTime();

        int idx = 0;
        for (int type : activeOrder) {
            glm::vec3 pos = base + glm::vec3(idx * SPACING, 0, 0);

            glm::mat4 m = glm::translate(glm::mat4(1), pos);
            m = glm::rotate(m, t * 0.8f, glm::vec3(0, 1, 0));
            m = glm::scale(m, glm::vec3(1.5f * ORDER_SCALE));

            if (type == 1) RenderMesh(meshes["resource_sphere"], shaders["VertexColor"], m);
            if (type == 2) RenderMesh(meshes["resource_cone"], shaders["VertexColor"], m);
            if (type == 3) RenderMesh(meshes["resource_cylinder"], shaders["VertexColor"], m);

            idx++;
        }
    }


    void Tema2::RenderTrainAssembly(const TrainComponent& t, bool isLocomotive)
    {
        glm::mat4 baseMatrix = glm::translate(glm::mat4(1), t.position);
        baseMatrix = glm::rotate(baseMatrix, t.rotation, glm::vec3(0, 1, 0));
        baseMatrix = glm::rotate(baseMatrix, RADIANS(-90), glm::vec3(0, 1, 0));

        baseMatrix = glm::scale(baseMatrix, glm::vec3(trainScale));

        float wheelRadius = 0.4f;
        float baseHeight = 0.5f;
        float yBaseCenter = wheelRadius + baseHeight / 2.0f + 0.4f;
        float yDeck = yBaseCenter + baseHeight / 2.0f;

        if (isLocomotive) {
            RenderMesh(meshes["box_yellow"], shaders["VertexColor"],
                glm::scale(glm::translate(baseMatrix, glm::vec3(2.4f, yBaseCenter + 1.2f, 0)), glm::vec3(10.2f, baseHeight, 3.6f)));

            RenderMesh(meshes["box_green"], shaders["VertexColor"],
                glm::scale(glm::translate(baseMatrix, glm::vec3(-0.5f, yDeck + 3.0f, 0)),
                    glm::vec3(4.2f, 3.6f, 3.6f)));

            glm::mat4 mEng = glm::translate(baseMatrix, glm::vec3(4.5f, yDeck + 2.6f, 0));
            mEng = glm::rotate(mEng, RADIANS(90), glm::vec3(0, 0, 1));
            RenderMesh(meshes["cylinder_blue"], shaders["VertexColor"], glm::scale(mEng, glm::vec3(2.6f, 6.0f, 2.6f)));

            glm::mat4 mNose = glm::translate(baseMatrix, glm::vec3(7.6f, yDeck + 2.4f, -0.1f));
            mNose = glm::rotate(mNose, RADIANS(90), glm::vec3(0, 0, 1));
            RenderMesh(meshes["cylinder_pink"], shaders["VertexColor"], glm::scale(mNose, glm::vec3(1.6f, 0.5f, 1.6f)));

            
            int wheelPairs = 6;                     
            float wheelRadiusL = 1.4f;             
            float wheelThickness = 0.3f;           
            float wheelY = wheelRadiusL;            

            float platformLen = 9.2f;               
            float platformWidth = 3.2f;             
            float sideOffset = platformWidth * 0.5f + wheelThickness * 0.55f; 

            
            float margin = 0.55f;                   

            
            for (int i = 0; i < wheelPairs; i++) {
                float t = (wheelPairs == 1) ? 0.5f : (float)i / (wheelPairs - 1);  
                float xPos = -platformLen * 0.5f + margin + t * (platformLen - 2.0f * margin) + 2.4f;

                glm::mat4 W1 = glm::translate(baseMatrix, glm::vec3(xPos, wheelY, +sideOffset));
                glm::mat4 W2 = glm::translate(baseMatrix, glm::vec3(xPos, wheelY, -sideOffset));

                
                W1 = glm::rotate(W1, RADIANS(90), glm::vec3(1, 0, 0));
                W2 = glm::rotate(W2, RADIANS(90), glm::vec3(1, 0, 0));

                RenderMesh(meshes["wheel_red"], shaders["VertexColor"],
                    glm::scale(W1, glm::vec3(wheelRadiusL, wheelThickness, wheelRadiusL)));
                RenderMesh(meshes["wheel_red"], shaders["VertexColor"],
                    glm::scale(W2, glm::vec3(wheelRadiusL, wheelThickness, wheelRadiusL)));
            }
        }
        else {
            RenderMesh(meshes["box_yellow"], shaders["VertexColor"],
                glm::scale(glm::translate(baseMatrix, glm::vec3(0, yBaseCenter, 0)), glm::vec3(4.8f, baseHeight, 1.4f)));

            RenderMesh(meshes["box_green"], shaders["VertexColor"],
                glm::scale(glm::translate(baseMatrix, glm::vec3(0, yDeck + 0.8f, 0)), glm::vec3(4.5f, 1.6f, 1.3f)));

            float wPosX[] = { -1.8f, 1.8f };
            for (int i = 0; i < 2; i++) {
                auto W1 = glm::translate(baseMatrix, glm::vec3(wPosX[i], wheelRadius, 0.72f));
                auto W2 = glm::translate(baseMatrix, glm::vec3(wPosX[i], wheelRadius, -0.72f));
                W1 = glm::rotate(W1, RADIANS(90), glm::vec3(1, 0, 0));
                W2 = glm::rotate(W2, RADIANS(90), glm::vec3(1, 0, 0));

                RenderMesh(meshes["wheel_red"], shaders["VertexColor"], glm::scale(W1, glm::vec3(0.8f, 0.2f, 0.8f)));
                RenderMesh(meshes["wheel_red"], shaders["VertexColor"], glm::scale(W2, glm::vec3(0.8f, 0.2f, 0.8f)));
            }
        }
    }

    Rail* Tema2::ChooseNextRail(const TrainComponent& t, DesiredDir desired)
    {
        if (!t.currentRail) return nullptr;

        glm::vec3 junction = t.forward ? t.currentRail->endPosition : t.currentRail->startPosition;

        glm::vec3 dirCur = t.forward ? (t.currentRail->endPosition - t.currentRail->startPosition)
            : (t.currentRail->startPosition - t.currentRail->endPosition);
        dirCur.y = 0;
        if (glm::length(dirCur) < 1e-6f) return nullptr;
        dirCur = glm::normalize(dirCur);

        auto signedAngle = [](const glm::vec3& a, const glm::vec3& b) -> float {
            float crossY = glm::cross(a, b).y;
            float dot = glm::clamp(glm::dot(a, b), -1.0f, 1.0f);
            return std::atan2(crossY, dot);
            };

        const float PI = 3.1415926535f;

        Rail* best = nullptr;
        float bestScore = 1e9f;

        for (Rail* cand : track) {
            if (!cand) continue;

            bool touchStart = glm::distance(cand->startPosition, junction) < 0.001f;
            bool touchEnd = glm::distance(cand->endPosition, junction) < 0.001f;
            if (!touchStart && !touchEnd) continue;


            auto samePoint = [](const glm::vec3& p, const glm::vec3& q) {
                return glm::distance(p, q) < 0.001f;
                };

            bool sameEdge =
                (samePoint(cand->startPosition, t.currentRail->startPosition) &&
                    samePoint(cand->endPosition, t.currentRail->endPosition)) ||
                (samePoint(cand->startPosition, t.currentRail->endPosition) &&
                    samePoint(cand->endPosition, t.currentRail->startPosition));

            if (sameEdge) continue;

            glm::vec3 dirN = touchStart ? (cand->endPosition - cand->startPosition)
                : (cand->startPosition - cand->endPosition);
            dirN.y = 0;
            if (glm::length(dirN) < 1e-6f) continue;
            dirN = glm::normalize(dirN);

            float ang = -signedAngle(dirCur, dirN);

            bool ok = false;
            float score = 9999.f;

            if (desired == DIR_STRAIGHT) {
                ok = std::abs(ang) < PI * 0.40f;
                score = std::abs(ang);
            }
            else if (desired == DIR_LEFT) {
                ok = ang < -PI * 0.15f && ang > -PI * 0.95f;
                score = std::abs(ang + PI * 0.5f);
            }
            else if (desired == DIR_RIGHT) {
                ok = ang > PI * 0.15f && ang < PI * 0.95f;
                score = std::abs(ang - PI * 0.5f);
            }
            else if (desired == DIR_BACK) {
                ok = std::abs(ang) > PI * 0.75f;
                score = std::abs(std::abs(ang) - PI);
            }

            if (ok && score < bestScore) {
                bestScore = score;
                best = cand;
            }
        }

        return best;
    }


    void Tema2::UpdateTrainMovement(TrainComponent& t, float deltaTime)
    {
        if (gameOver || !t.currentRail) return;

        float len = glm::distance(t.currentRail->startPosition, t.currentRail->endPosition);
        if (len > 0.0001f) {
            t.progress += (t.speed * deltaTime) / len;
        }

        while (t.progress >= 1.0f) {
            t.progress = 0.0f;

            glm::vec3 junction = t.forward
                ? t.currentRail->endPosition
                : t.currentRail->startPosition;

            std::vector<Rail*> exits;

            for (Rail* cand : track) {
                if (!cand) continue;

                bool touchStart = glm::distance(cand->startPosition, junction) < 0.001f;
                bool touchEnd = glm::distance(cand->endPosition, junction) < 0.001f;
                if (!touchStart && !touchEnd) continue;

                bool same =
                    (glm::distance(cand->startPosition, t.currentRail->startPosition) < 0.001f &&
                        glm::distance(cand->endPosition, t.currentRail->endPosition) < 0.001f) ||
                    (glm::distance(cand->startPosition, t.currentRail->endPosition) < 0.001f &&
                        glm::distance(cand->endPosition, t.currentRail->startPosition) < 0.001f);

                if (!same)
                    exits.push_back(cand);
            }

            Rail* next = nullptr;

            
            if (exits.empty()) {
                t.forward = !t.forward;
                desiredDir = DIR_STRAIGHT;
                break;
            }

            
            if (exits.size() == 1) {
                next = exits[0];
            }
            
            else {
                next = ChooseNextRail(t, desiredDir);

                if (!next) {
                    next = exits[0];
                }
            }

            if (glm::distance(next->startPosition, junction) < 0.001f)
                t.forward = true;
            else
                t.forward = false;

            t.currentRail = next;
            desiredDir = DIR_STRAIGHT;
        }

        glm::vec3 a = t.forward ? t.currentRail->startPosition : t.currentRail->endPosition;
        glm::vec3 b = t.forward ? t.currentRail->endPosition : t.currentRail->startPosition;

        t.position = glm::mix(a, b, t.progress);

        glm::vec3 dir = glm::normalize(b - a);
        t.rotation = std::atan2(dir.x, dir.z);
    }


    void Tema2::UpdateStationsRespawn(float dt)
    {
        for (auto& s : stations) {
            if (s.resourceType == 0) continue;

            if (!s.resourceAvailable) {
                s.respawnTimer -= dt;
                if (s.respawnTimer <= 0.f) {
                    s.respawnTimer = 0.f;
                    s.resourceAvailable = true;
                }
            }
        }
    }

    void Tema2::HandleStationInteractions()
    {
        AABB locoBox = GetLocomotiveAABB();

        int hitIdx = -1;

        for (int i = 0; i < (int)stations.size(); i++) {
            Station& s = stations[i];

            AABB stBox = GetStationAABB(s);
            bool hitStation = IntersectAABB(locoBox, stBox);

            bool hitResource = false;
            if (s.resourceType != 0 && s.resourceAvailable) {
                AABB resBox = GetResourceAABB(s);
                hitResource = IntersectAABB(locoBox, resBox);
            }

            if (hitStation || hitResource) {
                hitIdx = i;
                break;
            }
        }

        if (hitIdx == -1) {
            lastStationIdx = -1;
            return;
        }

        if (hitIdx == lastStationIdx) return;
        lastStationIdx = hitIdx;

        Station& s = stations[hitIdx];

        if (s.resourceType == 0) {
            if (activeOrder.empty()) {
                score += 100;
                std::cout << ">>> COMANDA LIVRATA! Scor: " << score << "\n";
                GenerateOrder();
            }
            else {
                std::cout << "INFO: La baza, dar mai ai " << activeOrder.size()
                    << " resurse (in ordine) de colectat.\n";
            }
            return;
        }

        if (!activeOrder.empty() && s.resourceAvailable && activeOrder.front() == s.resourceType) {
            std::cout << "Colectat resursa " << s.resourceType
                << " (corect). Mai ai " << (activeOrder.size() - 1) << "\n";

            activeOrder.pop_front();

            s.resourceAvailable = false;
            s.respawnTimer = 5.0f;
        }
        else {
            if (!activeOrder.empty() && activeOrder.front() != s.resourceType) {
                std::cout << "GRESIT: urmatoarea resursa ceruta e tip "
                    << activeOrder.front() << "\n";
            }
        }
    }


    void Tema2::RenderMinimap(float timeFactor)
    {
        auto res = window->GetResolution();

        int w = (int)res.x;
        int h = (int)res.y;

        int vpW = (int)(w * 0.22f);
        int vpH = (int)(h * 0.22f);

        int vpX = w - vpW - 40;
        int vpY = 15;

        glClear(GL_DEPTH_BUFFER_BIT);
        glViewport(vpX, vpY, vpW, vpH);

        implemented::Camera* savedCam = camera;
        glm::mat4 savedProj = projectionMatrix;

        camera = minimapCamera;
        projectionMatrix = orthoMatrix;

        RenderTerrain();
        RenderRails();
        RenderStations(timeFactor);

        RenderTrainAssembly(locomotive, true);
        for (const auto& w : wagons) {
            RenderTrainAssembly(w, false);
        }

        camera = savedCam;
        projectionMatrix = savedProj;

        glViewport(0, 0, w, h);
    }

    void Tema2::Update(float deltaTimeSeconds)
    {
        if (!gameOver) {
            timeRemaining -= deltaTimeSeconds;
            if (timeRemaining <= 0.f) {
                timeRemaining = 0.f;
                gameOver = true;
                gameOverStartTime = (float)glfwGetTime();
                std::cout << "GAME OVER! Nu ai terminat comanda la timp. Scor: " << score << "\n";
            }
        }

        float timeFactor = 0.f;
        if (maxTime > 0.f) timeFactor = 1.0f - (timeRemaining / maxTime);

        UpdateStationsRespawn(deltaTimeSeconds);

        UpdateTrainMovement(locomotive, deltaTimeSeconds);

        trainHistoryPos.push_front(locomotive.position);
        trainHistoryRot.push_front(locomotive.rotation);

        
        int needed = wagonDelayFrames * std::max(1, numWagons) + 5;

        
        while ((int)trainHistoryPos.size() > needed) trainHistoryPos.pop_back();
        while ((int)trainHistoryRot.size() > needed) trainHistoryRot.pop_back();

        if (gameOver) {
            float elapsed = (float)glfwGetTime() - gameOverStartTime;

            if (elapsed >= gameOverDuration) {
                window->Close();
                return;
            }
        }

        
        for (int i = 0; i < (int)wagons.size(); i++) {
            int idx = wagonDelayFrames * (i + 1);

            if ((int)trainHistoryPos.size() > idx) {
                wagons[i].position = trainHistoryPos[idx];
                wagons[i].rotation = trainHistoryRot[idx];
            }
            else {
                wagons[i].position = locomotive.position;
                wagons[i].rotation = locomotive.rotation;
            }
        }


        if (!gameOver) {
            HandleStationInteractions();
        }

        RenderTerrain();
        RenderRails();
        RenderIntersectionPoles();
        RenderStations(timeFactor);

        RenderOrderUI();

        RenderTrainAssembly(locomotive, true);
        for (const auto& w : wagons) {
            RenderTrainAssembly(w, false);
        }

        RenderMinimap(timeFactor);

        RenderTimerUI();

        if (gameOver) {
            RenderGameOverUI();
            return;
        }
    }

    void Tema2::OnInputUpdate(float deltaTime, int mods)
    {
        if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {
            float speed = 30.0f;

            glm::vec3 forward = glm::normalize(glm::vec3(camera->forward.x, 0.0f, camera->forward.z));
            glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
            glm::vec3 up = glm::vec3(0, 1, 0);

            if (window->KeyHold(GLFW_KEY_UP))   camera->position += forward * (deltaTime * speed);
            if (window->KeyHold(GLFW_KEY_DOWN)) camera->position -= forward * (deltaTime * speed);

            if (window->KeyHold(GLFW_KEY_LEFT))  camera->position -= right * (deltaTime * speed);
            if (window->KeyHold(GLFW_KEY_RIGHT)) camera->position += right * (deltaTime * speed);

            if (window->KeyHold(GLFW_KEY_E)) camera->position += up * (deltaTime * speed);
            if (window->KeyHold(GLFW_KEY_Q)) camera->position -= up * (deltaTime * speed);
        }
    }


    void Tema2::OnKeyPress(int key, int mods)
    {
        if (key == GLFW_KEY_W) desiredDir = DIR_STRAIGHT;
        if (key == GLFW_KEY_A) desiredDir = DIR_LEFT;
        if (key == GLFW_KEY_D) desiredDir = DIR_RIGHT;
        if (key == GLFW_KEY_S) desiredDir = DIR_BACK;
    }

    void Tema2::OnKeyRelease(int key, int mods) {}

    void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
    {
        if (window->MouseHold(GLFW_MOUSE_BUTTON_RIGHT)) {
            camera->RotateFirstPerson_OX(-deltaY * 0.002f);
            camera->RotateFirstPerson_OY(-deltaX * 0.002f);
        }
    }

    void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) {}
    void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) {}
    void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) {}

    void Tema2::OnWindowResize(int width, int height)
    {
        if (!textRenderer) return;

        delete textRenderer;
        textRenderer = new gfxc::TextRenderer(window->props.selfDir, (GLuint)width, (GLuint)height);
        textRenderer->Load("assets/fonts/Hack-Bold.ttf", 32);
    }


    void Tema2::FrameStart()
    {
        glClearColor(C_IARBA.r, C_IARBA.g, C_IARBA.b, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::ivec2 r = window->GetResolution();
        glViewport(0, 0, r.x, r.y);
    }

    void Tema2::FrameEnd() {}

    
    void Tema2::CreateCube(const char* name, glm::vec3 color)
    {
        std::vector<VertexFormat> v = {
            VertexFormat(glm::vec3(-0.5,-0.5, 0.5), color),
            VertexFormat(glm::vec3(0.5,-0.5, 0.5), color),
            VertexFormat(glm::vec3(-0.5, 0.5, 0.5), color),
            VertexFormat(glm::vec3(0.5, 0.5, 0.5), color),
            VertexFormat(glm::vec3(-0.5,-0.5,-0.5), color),
            VertexFormat(glm::vec3(0.5,-0.5,-0.5), color),
            VertexFormat(glm::vec3(-0.5, 0.5,-0.5), color),
            VertexFormat(glm::vec3(0.5, 0.5,-0.5), color)
        };

        std::vector<unsigned int> i = {
            0,1,2, 1,3,2,
            2,3,7, 2,7,6,
            1,7,3, 1,5,7,
            6,7,4, 7,5,4,
            0,4,1, 1,4,5,
            2,6,4, 0,2,4
        };

        CreateMesh(name, v, i);
    }

    void Tema2::CreateCylinder(const char* name, glm::vec3 color)
    {
        std::vector<VertexFormat> v;
        std::vector<unsigned int> idx;

        int slices = 30;
        float h = 1.0f, r = 0.5f;

        v.emplace_back(glm::vec3(0, -h / 2, 0), color);
        v.emplace_back(glm::vec3(0, h / 2, 0), color);

        for (int k = 0; k < slices; k++) {
            float a = 2 * 3.14159f * k / slices;
            v.emplace_back(glm::vec3(r * std::cos(a), -h / 2, r * std::sin(a)), color);
            v.emplace_back(glm::vec3(r * std::cos(a), h / 2, r * std::sin(a)), color);
        }

        for (int k = 0; k < slices; k++) {
            int c = 2 + k * 2;
            int n = 2 + ((k + 1) % slices) * 2;

            idx.push_back(0); idx.push_back(n); idx.push_back(c);
            idx.push_back(1); idx.push_back(c + 1); idx.push_back(n + 1);

            idx.push_back(c); idx.push_back(n); idx.push_back(n + 1);
            idx.push_back(c); idx.push_back(n + 1); idx.push_back(c + 1);
        }

        CreateMesh(name, v, idx);
    }

    void Tema2::CreateSphere(const char* name, glm::vec3 color)
    {
        std::vector<VertexFormat> v;
        std::vector<unsigned int> idx;

        int lat = 20, lon = 20;
        float r = 0.5f;

        for (int i = 0; i <= lat; i++) {
            float theta = i * 3.14159f / lat;
            float sinT = std::sin(theta), cosT = std::cos(theta);

            for (int j = 0; j <= lon; j++) {
                float phi = j * 2 * 3.14159f / lon;
                float x = r * sinT * std::cos(phi);
                float y = r * cosT;
                float z = r * sinT * std::sin(phi);
                v.emplace_back(glm::vec3(x, y, z), color);
            }
        }

        for (int i = 0; i < lat; i++) {
            for (int j = 0; j < lon; j++) {
                int f = (i * (lon + 1)) + j;
                int s = f + lon + 1;

                idx.push_back(f); idx.push_back(s); idx.push_back(f + 1);
                idx.push_back(s); idx.push_back(s + 1); idx.push_back(f + 1);
            }
        }

        CreateMesh(name, v, idx);
    }

    void Tema2::CreateCone(const char* name, glm::vec3 color)
    {
        std::vector<VertexFormat> v;
        std::vector<unsigned int> idx;

        int s = 30;
        float h = 1.0f, r = 0.5f;

        v.emplace_back(glm::vec3(0, h / 2, 0), color);
        v.emplace_back(glm::vec3(0, -h / 2, 0), color);

        for (int k = 0; k < s; k++) {
            float a = 2 * 3.14159f * k / s;
            v.emplace_back(glm::vec3(r * std::cos(a), -h / 2, r * std::sin(a)), color);
        }

        for (int k = 0; k < s; k++) {
            int c = 2 + k;
            int nc = 2 + (k + 1) % s;

            idx.push_back(0); idx.push_back(nc); idx.push_back(c);
            idx.push_back(1); idx.push_back(c); idx.push_back(nc);
        }

        CreateMesh(name, v, idx);
    }

    Mesh* Tema2::CreateMesh(const char* name,
        const std::vector<VertexFormat>& vertices,
        const std::vector<unsigned int>& indices)
    {
        unsigned int VAO = 0, VBO = 0, IBO = 0;

        glGenVertexArrays(1, &VAO);
        glBindVertexArray(VAO);

        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

        glGenBuffers(1, &IBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), &indices[0], GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat), (void*)0);

        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(VertexFormat),
            (void*)(2 * sizeof(glm::vec3) + sizeof(glm::vec2)));

        glBindVertexArray(0);

        if (meshes[name]) delete meshes[name];
        meshes[name] = new Mesh(name);
        meshes[name]->InitFromBuffer(VAO, (unsigned int)indices.size());

        return meshes[name];
    }


    bool Tema2::IntersectAABB(const AABB& a, const AABB& b)
    {
        return (a.min.x <= b.max.x && a.max.x >= b.min.x) &&
            (a.min.y <= b.max.y && a.max.y >= b.min.y) &&
            (a.min.z <= b.max.z && a.max.z >= b.min.z);
    }

    AABB Tema2::MakeAABB(const glm::vec3& center, const glm::vec3& halfExtents)
    {
        AABB box;
        box.min = center - halfExtents;
        box.max = center + halfExtents;
        return box;
    }

    AABB Tema2::GetLocomotiveAABB() const
    {
        glm::vec3 half = glm::vec3(6.2f, 2.0f, 1.8f) * (trainScale * 0.5f);
        glm::vec3 center = locomotive.position + glm::vec3(0, 1.2f * trainScale, 0);
        return MakeAABB(center, half);
    }

    AABB Tema2::GetStationAABB(const Station& s) const
    {
        const float STATION_SCALE = 1.6f;
        glm::vec3 half = glm::vec3(14.0f, 6.0f, 14.0f) * (STATION_SCALE * 0.5f);
        glm::vec3 center = s.position + glm::vec3(0, 2.8f * STATION_SCALE, 0);
        return MakeAABB(center, half);
    }

    AABB Tema2::GetResourceAABB(const Station& s) const
    {
        const float STATION_SCALE = 1.6f;
        glm::vec3 center = s.position + glm::vec3(0, 6.0f * STATION_SCALE, 0);
        glm::vec3 half = glm::vec3(5.0f, 3.0f, 5.0f) * (STATION_SCALE * 0.5f);
        return MakeAABB(center, half);
    }


    void Tema2::DetectIntersections()
    {
        intersections.clear();

        std::vector<glm::vec3> points;
        std::vector<int> counts;

        auto samePoint = [](const glm::vec3& a, const glm::vec3& b) {
            return glm::distance(a, b) < 0.001f;
            };

        for (auto r : track) {
            glm::vec3 pts[2] = { r->startPosition, r->endPosition };

            for (auto& p : pts) {
                bool found = false;
                for (int i = 0; i < (int)points.size(); i++) {
                    if (samePoint(points[i], p)) {
                        counts[i]++;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    points.push_back(p);
                    counts.push_back(1);
                }
            }
        }

        for (int i = 0; i < (int)points.size(); i++) {
            if (counts[i] >= 3) {
                intersections.push_back(points[i]);
            }
        }
    }


    void Tema2::RenderIntersectionPoles()
    {
        const float POLE_SCALE = 2.0f;

        const float DIST_FROM_RAILS = 5.0f;

        auto samePoint = [](const glm::vec3& a, const glm::vec3& b) {
            return glm::distance(a, b) < 0.001f;
            };

        auto perpXZ = [](const glm::vec3& d) {
            return glm::vec3(-d.z, 0.0f, d.x);
            };

        for (const auto& p : intersections) {

            std::vector<glm::vec3> tangents;

            for (Rail* r : track) {
                if (!r) continue;

                if (samePoint(r->startPosition, p)) {
                    glm::vec3 d = r->endPosition - r->startPosition;
                    d.y = 0.0f;
                    if (glm::length(d) > 1e-6f) tangents.push_back(glm::normalize(d));
                }
                else if (samePoint(r->endPosition, p)) {
                    glm::vec3 d = r->startPosition - r->endPosition;
                    d.y = 0.0f;
                    if (glm::length(d) > 1e-6f) tangents.push_back(glm::normalize(d));
                }
            }

            glm::vec3 toCenter = glm::vec3(-p.x, 0.0f, -p.z);

            if (glm::length(toCenter) < 1e-6f) {
                toCenter = glm::vec3(1.0f, 0.0f, 1.0f);
            }
            toCenter = glm::normalize(toCenter);

            glm::vec3 offsetDir(0.0f);

            if (!tangents.empty()) {
                for (const auto& t : tangents) {
                    glm::vec3 n = perpXZ(t);
                    if (glm::length(n) < 1e-6f) continue;
                    n = glm::normalize(n);

                    if (glm::dot(n, toCenter) < 0.0f) n = -n;

                    offsetDir += n;
                }
            }

            if (glm::length(offsetDir) < 1e-6f) {
                offsetDir = toCenter;
            }
            else {
                offsetDir = glm::normalize(offsetDir);
            }

            
            glm::vec3 pos = p + offsetDir * (DIST_FROM_RAILS * POLE_SCALE);

            
            glm::mat4 base = glm::translate(glm::mat4(1), pos);
            base = glm::scale(base, glm::vec3(POLE_SCALE));

            
            {
                glm::mat4 m = glm::translate(base, glm::vec3(0, 0.20f, 0));
                m = glm::scale(m, glm::vec3(1.6f, 0.40f, 1.6f));
                RenderMesh(meshes["box_concrete_dark"], shaders["VertexColor"], m);
            }
            {
                glm::mat4 m = glm::translate(base, glm::vec3(0, 0.55f, 0));
                m = glm::scale(m, glm::vec3(1.0f, 0.30f, 1.0f));
                RenderMesh(meshes["box_concrete"], shaders["VertexColor"], m);
            }

            
            const float poleH = 6.2f;
            const float segH = 0.45f;
            const float poleW = 0.30f;
            const int segCount = (int)(poleH / segH);

            for (int i = 0; i < segCount; i++) {
                float y = 0.85f + i * segH;

                glm::mat4 m = glm::translate(base, glm::vec3(0, y, 0));
                m = glm::scale(m, glm::vec3(poleW, segH * 0.5f, poleW));

                if (i % 2 == 0) RenderMesh(meshes["box_white"], shaders["VertexColor"], m);
                else            RenderMesh(meshes["box_red"], shaders["VertexColor"], m);
            }

            
            {
                glm::mat4 m = glm::translate(base, glm::vec3(0, 6.0f, 0));
                m = glm::rotate(m, RADIANS(35.0f), glm::vec3(0, 1, 0));
                m = glm::rotate(m, RADIANS(35.0f), glm::vec3(0, 0, 1));
                m = glm::scale(m, glm::vec3(2.2f, 0.18f, 0.35f));
                RenderMesh(meshes["box_white"], shaders["VertexColor"], m);
            }
            {
                glm::mat4 m = glm::translate(base, glm::vec3(0, 6.0f, 0));
                m = glm::rotate(m, RADIANS(-35.0f), glm::vec3(0, 1, 0));
                m = glm::rotate(m, RADIANS(-35.0f), glm::vec3(0, 0, 1));
                m = glm::scale(m, glm::vec3(2.2f, 0.18f, 0.35f));
                RenderMesh(meshes["box_white"], shaders["VertexColor"], m);
            }

            
            {
                glm::mat4 m = glm::translate(base, glm::vec3(0, 6.9f, 0));
                m = glm::scale(m, glm::vec3(0.55f, 0.35f, 0.55f));
                RenderMesh(meshes["box_red"], shaders["VertexColor"], m);
            }

            
            {
                glm::vec3 lampBaseLocal = glm::vec3(0, 5.2f, 0.45f);

                for (int k = 0; k < 2; k++) {
                    float xOff = (k == 0) ? -0.35f : 0.35f;

                    glm::mat4 m = glm::translate(base, lampBaseLocal + glm::vec3(xOff, 0, 0));
                    m = glm::rotate(m, RADIANS(90.0f), glm::vec3(1, 0, 0));
                    m = glm::scale(m, glm::vec3(0.35f, 0.10f, 0.35f));
                    RenderMesh(meshes["wheel_red"], shaders["VertexColor"], m);
                }
            }
        }
    }


    void Tema2::RenderTimerUI()
    {
        if (!textRenderer) return;

        glm::ivec2 res = window->GetResolution();

        
        float scale = 1.2f * (res.y / 720.0f);     
        float marginX = 20.0f * scale;
        float marginY = 20.0f * scale;

        int secondsLeft = (int)std::ceil(timeRemaining);
        if (secondsLeft < 0) secondsLeft = 0;

        std::string txt = "Time Left: " + std::to_string(secondsLeft);

        
        float x = marginX + 100.0f;
        float y = marginY + 100.0f;

        
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        textRenderer->RenderText(txt, x, y, scale, glm::vec3(0.0f, 0.0f, 0.0f));

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }


    void Tema2::RenderGameOverUI()
    {
        if (!textRenderer) return;

        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::ivec2 res = window->GetResolution();

        
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        

        float bigScale = 3.2f * (res.y / 720.0f);
        float smallScale = 1.4f * (res.y / 720.0f);

        
        float xTitle = res.x * 0.30f;   
        float yTitle = res.y * 0.20f;   

        float xMsg = res.x * 0.22f;
        float yMsg = res.y * 0.32f;

        float elapsed = (float)glfwGetTime() - gameOverStartTime;
        int left = (int)std::ceil(gameOverDuration - elapsed);
        if (left < 0) left = 0;

        std::string title = "GAME OVER";
        std::string msg1 = "Time is up!";
        std::string msg2 = "Closing in " + std::to_string(left) + " ...";

        
        textRenderer->RenderText(title, xTitle, yTitle, bigScale, glm::vec3(0.9f, 0.1f, 0.1f));

        
        textRenderer->RenderText(msg1, xMsg, yMsg, smallScale, glm::vec3(1.0f, 1.0f, 1.0f));
        textRenderer->RenderText(msg2, xMsg, yMsg + 40.0f * smallScale, smallScale, glm::vec3(1.0f, 1.0f, 1.0f));

        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }
} // namespace m1

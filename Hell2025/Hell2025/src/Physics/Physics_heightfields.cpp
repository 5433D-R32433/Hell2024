#include "Physics.h"
#include "Physics/Types/HeightField.h"
#include <vector>

#include "Core/Game.h"
#include "Editor/Editor.h"
#include "Util/Util.h"


#include "Renderer/Renderer.h"

namespace Physics {

    std::vector<HeightField> g_HeightFields;

    void UpdateHeightFields() {

        float threshold = 6.0f;

        for (HeightField& heightfield : g_HeightFields) {
            const AABB& heightFieldAABB = heightfield.GetAABB();

            bool intersectionFound = false;
            float threshold = 0.25f;

            // Enable heightfield physics if other active PhysX object AABBs intersect heightfield AABB
            if (Editor::IsEditorClosed()) {

                // Players
                for (int i = 0; i < Game::GetLocalPlayerCount(); i++) {
                    const AABB& playerAABB = Game::GetLocalPlayerByIndex(i)->GetCharacterControllerAABB();
                    if (heightFieldAABB.IntersectsAABB(playerAABB, threshold)) {
                        intersectionFound = true;
                        break;
                    }
                }

                // Active rigid dynamics
                if (!intersectionFound) {
                    const std::vector<AABB>& activeRigidAABBS = GetActiveRididDynamicAABBs();
                    for (const AABB& aabb : activeRigidAABBS) {

                        //Renderer::DrawAABB(aabb, GREEN);

                        if (heightFieldAABB.IntersectsAABB(aabb, threshold)) {
                            intersectionFound = true;
                            break;
                        }
                    }
                }
            }

            if (intersectionFound) {
                heightfield.ActivatePhsyics();
            }
            else {
                heightfield.DisablePhsyics();
            }
        }
    }

    void RemoveAnyHeightFieldMarkedForRemoval() {
        PxScene* pxScene = Physics::GetPxScene();

        //std::cout << g_HeightFields.size() << "\n";

        for (int i = 0; i < g_HeightFields.size(); i++) {
            HeightField& heightField = g_HeightFields[i];

            if (heightField.IsMarkedForRemoval()) {
                PxHeightField* pxHeightField = heightField.GetPxHeightField();
                PxRigidStatic* pxRigidStatic = heightField.GetPxRigidStatic();
                PxShape* pxShape = heightField.GetPxShape();

                 // Remove the actor from the scene 
                if (pxRigidStatic && heightField.HasActivePhysics()) {
                    pxScene->removeActor(*pxRigidStatic);
                }

                // Release the shape
                if (pxShape) {
                    pxShape->release();
                    pxShape = nullptr;
                }

                // Release the height field
                if (pxHeightField) {
                    pxHeightField->release();
                    pxHeightField = nullptr;
                }

                 // Finally, release the rigid static actor
                if (pxRigidStatic) {

                    // Clean up user data
                    if (pxRigidStatic->userData) {
                        delete static_cast<PhysicsUserData*>(pxRigidStatic->userData);
                        pxRigidStatic->userData = nullptr;
                    }

                    pxRigidStatic->release();
                    pxRigidStatic = nullptr;
                }

                // Remove from container
                g_HeightFields.erase(g_HeightFields.begin() + i);
                i--;
            }
        }
    }

    void MarkAllHeightFieldsForRemoval() {
        for (HeightField& heightField : g_HeightFields) {
            heightField.MarkForRemoval();
        }
    }

    void CreateHeightField(vecXZ& worldSpaceOffset, const float* heightValues) {
        HeightField& g_heightFields = g_HeightFields.emplace_back();
        g_heightFields.Create(worldSpaceOffset, heightValues);
    }
}
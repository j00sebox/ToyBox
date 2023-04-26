#pragma once

#include "Component.h"
#include "renderer/Fwd.h"

class MaterialComponent final : public Component
{
public:
    MaterialComponent(Material&& material);
    MaterialComponent(std::shared_ptr<Material> material_ptr);
    [[nodiscard]] const Material& get() const { return m_material.operator*(); }
    [[nodiscard]] std::shared_ptr<Material> get_ptr() { return m_material; }

    [[nodiscard]] const char* get_name() const override { return "Material"; }
    [[nodiscard]] size_t get_type() const override { return typeid(MaterialComponent).hash_code(); }
    void imgui_render() override;
    void serialize(nlohmann::json& accessor) const override;

private:
    std::shared_ptr<Material> m_material;
};


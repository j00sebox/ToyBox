#include "pch.h"
#include "Renderer.h"
#include "GLError.h"
#include "Shader.h"
#include "Mesh.h"
#include "components/MeshObject.h"
#include "components/Material.h"
#include "components/Transform.h"

#include <glad/glad.h>

void Renderer::init(int width, int height)
{
	set_viewport(width, height);

    GL_CALL(glEnable(GL_MULTISAMPLE));
    GL_CALL(glEnable(GL_CULL_FACE));
    GL_CALL(glEnable(GL_BLEND));
    GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
	GL_CALL(glEnable(GL_STENCIL_TEST));
	GL_CALL(glEnable(GL_DEPTH_TEST));
	GL_CALL(glDepthFunc(GL_LEQUAL));
    GL_CALL(glEnable(GL_FRAMEBUFFER_SRGB));

    GL_CALL(glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP));
    GL_CALL(glStencilFunc(GL_NOTEQUAL, 1, 0xFF));
    GL_CALL(glStencilMask(0xFF));
}

void Renderer::set_viewport(int width, int height)
{
	GL_CALL(glViewport(0, 0, width, height));
}

void Renderer::set_clear_colour(glm::vec4 colour)
{
	GL_CALL(glClearColor(colour.x, colour.y, colour.z, colour.w));
}

void Renderer::draw_elements(const Transform& transform, const MeshObject& mesh, const Material& material)
{
    material.get_shader()->set_uniform_mat4f("u_model", transform.get_transform());
    material.get_shader()->set_uniform_4f("u_flat_colour", material.get_colour());

	material.bind();
	mesh.bind();

	GL_CALL(glDrawElements(GL_TRIANGLES, mesh.get_mesh()->get_index_count(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::draw_elements_instanced(unsigned int instances, const MeshObject& mesh_obj, const Material& material)
{
    material.bind();
    mesh_obj.bind();
    GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, mesh_obj.get_mesh()->get_index_count(), GL_UNSIGNED_INT, nullptr, instances));
}

void Renderer::draw_skybox(const Skybox& skybox)
{
    GL_CALL(glDisable(GL_CULL_FACE));
    GL_CALL(glDepthMask(GL_FALSE));
    skybox.bind();
    GL_CALL(glDrawElements(GL_TRIANGLES, skybox.get_indices(), GL_UNSIGNED_INT, nullptr));
    GL_CALL(glDepthMask(GL_TRUE));
    skybox.unbind();
    GL_CALL(glEnable(GL_CULL_FACE));
}

void Renderer::stencil(const Transform& stencil_transform, const MeshObject& mesh, const Material& material)
{
    GL_CALL(glStencilOp(GL_KEEP, GL_REPLACE, GL_REPLACE));
	GL_CALL(glStencilFunc(GL_ALWAYS, 2, 0xFF)); // make all the fragments of the object have a stencil of 1
	
	material.bind();
	mesh.bind();
	GL_CALL(glDrawElements(GL_TRIANGLES, mesh.get_mesh()->get_index_count(), GL_UNSIGNED_INT, nullptr));

    ShaderTable::get("flat_colour")->set_uniform_mat4f("u_model", stencil_transform.get_transform());
    ShaderTable::get("flat_colour")->set_uniform_4f("u_flat_colour", {1.f, 1.f, 0.f, 1.f});
	ShaderTable::get("flat_colour")->bind();
    GL_CALL(glStencilOp(GL_KEEP, GL_KEEP, GL_INCR));
	GL_CALL(glStencilFunc(GL_NOTEQUAL, 2, 0xFF)); // now all fragments not apart of the original object are written

	GL_CALL(glDrawElements(GL_TRIANGLES, mesh.get_mesh()->get_index_count(), GL_UNSIGNED_INT, nullptr));
	
	// set back to normal for other objects
    GL_CALL(glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP));
	GL_CALL(glStencilFunc(GL_NOTEQUAL, 1, 0xFF));

}

void Renderer::shadow_pass(const std::vector<RenderObject> &render_list, unsigned int shadow_width, unsigned int shadow_height, bool using_cubemap)
{
    GL_CALL(glClear(GL_DEPTH_BUFFER_BIT));
    GLint viewport_size[4];
    glGetIntegerv( GL_VIEWPORT, viewport_size );

    int original_width = viewport_size[2];
    int original_height = viewport_size[3];

    glViewport(0, 0, shadow_width, shadow_height);
    for(const auto& render_obj : render_list)
    {
        if(render_obj.mesh->get_mesh()->is_instanced())
        {
            ShaderTable::get("inst_shadow_map")->bind();
            render_obj.mesh->bind();
            GL_CALL(glDrawElementsInstanced(GL_TRIANGLES, render_obj.mesh->get_mesh()->get_index_count(), GL_UNSIGNED_INT, nullptr, render_obj.instances));
        }
        else
        {
            if(using_cubemap)
            {
                ShaderTable::get("shadow_cubemap")->set_uniform_mat4f("u_model", render_obj.transform.get_transform());
                ShaderTable::get("shadow_cubemap")->bind();
            }
            else
            {
                ShaderTable::get("shadow_map")->set_uniform_mat4f("u_model", render_obj.transform.get_transform());
                ShaderTable::get("shadow_map")->bind();
            }

            render_obj.mesh->bind();
            GL_CALL(glDrawElements(GL_TRIANGLES, render_obj.mesh->get_mesh()->get_index_count(), GL_UNSIGNED_INT, nullptr));
        }
    }
    glViewport(0, 0, original_width, original_height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void Renderer::render_pass(const std::vector<RenderObject>& render_list)
{
   for(const auto& render_obj : render_list)
   {
        switch(render_obj.render_command)
        {
            case RenderCommand::ElementDraw:
            {
                draw_elements(render_obj.transform, *render_obj.mesh, *render_obj.material);
                break;
            }

            case RenderCommand::InstancedElementDraw:
            {
                draw_elements_instanced(render_obj.instances, *render_obj.mesh, *render_obj.material);
                break;
            }

            case RenderCommand::Stencil:
            {
                render_obj.material->get_shader()->set_uniform_mat4f("u_model", render_obj.transform.get_transform());
                render_obj.material->get_shader()->set_uniform_4f("u_base_colour", render_obj.material->get_colour());
                Transform stencil_transform = render_obj.transform;

                if(render_obj.mesh->is_using_scale_outline())
                {
                    ShaderTable::get("flat_colour")->set_uniform_1f("u_outlining_factor", 0.f);
                    stencil_transform.scale(stencil_transform.get_uniform_scale() * (1.f + render_obj.mesh->get_scale_outline_factor())); // scale up a tiny bit to see outline
                }
                else
                    ShaderTable::get("flat_colour")->set_uniform_1f("u_outlining_factor", render_obj.mesh->get_scale_outline_factor());

                stencil(stencil_transform, *render_obj.mesh, *render_obj.material);
                break;
            }
        }
   }
}

void Renderer::clear()
{
	GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT));
}



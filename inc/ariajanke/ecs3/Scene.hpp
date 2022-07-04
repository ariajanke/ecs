/****************************************************************************

    MIT License

    Copyright (c) 2022 Aria Janke

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*****************************************************************************/

#pragma once

#include <vector>
#include <algorithm>

#include <common/Util.hpp>

#include <ariajanke/ecs3/detail/EntityRef.hpp>

namespace ecs {

template <typename EntityType>
class SceneOf final {
public:
    using ConstIterator = typename std::vector<EntityType>::const_iterator;

    void update_entities()
        { m_real_home_scene.update_entities(); }

    void add_entity(const EntityType & ent)
        { set_home_scene_for(m_real_home_scene.add_entity(ent)); }

    void add_entities(const std::vector<EntityType> & vec)
        { set_home_scene_for(m_real_home_scene.add_entities(vec)); }

    EntityType make_entity() {
        auto rv = EntityType::make_sceneless_entity();
        add_entity(rv);
        return rv;
    }

    ConstIterator begin() const { return m_real_home_scene.begin(); }

    ConstIterator end() const { return m_real_home_scene.end(); }

    void clear()
        { m_real_home_scene.clear(); }

    auto count() const noexcept { return end() - begin(); }

private:
    using Iterator = typename std::vector<EntityType>::iterator;
    using IteratorView = cul::View<Iterator>;

    void set_home_scene_for(IteratorView view) {
        for (auto & new_ent : view) {
            new_ent.set_home_scene(m_real_home_scene);
        }
    }

    class HomeSceneComplete final : public HomeSceneBase<EntityType> {
    public:
        void on_create(const EntityType &) final;

        void on_deletion_request(const EntityType &) final;

        void update_entities();

        IteratorView add_entity(const EntityType &);

        IteratorView add_entities(const std::vector<EntityType> &);

        ConstIterator begin() const { return m_active_entities.begin(); }

        ConstIterator end() const { return m_active_entities.end(); }

        void clear();

        static bool compare_entities(const EntityType & lhs, const EntityType & rhs)
            { return lhs.hash() < rhs.hash(); }

    private:
        std::vector<EntityType> m_new_entities;
        std::vector<EntityType> m_active_entities;
        std::vector<EntityType> m_to_remove_entities;
    };

    HomeSceneComplete m_real_home_scene;
};


template <typename EntityType>
void SceneOf<EntityType>::HomeSceneComplete::update_entities() {
    for (auto * cont : { &m_active_entities, &m_to_remove_entities }) {
        std::sort(cont->begin(), cont->end(), compare_entities);
    }
    {
    auto rmitr = m_to_remove_entities.begin();
    auto cnitr = m_active_entities.begin();
    while (rmitr != m_to_remove_entities.end()) {
        assert(cnitr != m_active_entities.end());
        if (*cnitr == *rmitr) {
            *rmitr = *cnitr = EntityType{};
            ++rmitr;
        }
        ++cnitr;
    }
    m_active_entities.erase(
        std::remove(m_active_entities.begin(), m_active_entities.end(), EntityType{}),
        m_active_entities.end());
    m_to_remove_entities.clear();
    }
    {
    auto old_size = m_active_entities.size();
    m_active_entities.resize(old_size*2 + m_new_entities.size());
    std::merge(
        m_active_entities.begin(), m_active_entities.begin() + old_size,
        m_new_entities.begin(), m_new_entities.end(),
        m_active_entities.begin() + old_size,
        compare_entities);
    m_active_entities.erase(m_active_entities.begin(), m_active_entities.begin() + old_size);
    m_new_entities.clear();
    }
}

template <typename EntityType>
typename SceneOf<EntityType>::IteratorView
    SceneOf<EntityType>::HomeSceneComplete::add_entity(const EntityType & ent)
{
    m_active_entities.push_back(ent);
    return IteratorView{ m_active_entities.end() - 1, m_active_entities.end() };
}

template <typename EntityType>
typename SceneOf<EntityType>::IteratorView
    SceneOf<EntityType>::HomeSceneComplete::add_entities
    (const std::vector<EntityType> & vec)
{
    auto old_size = m_active_entities.size();
    m_active_entities.insert(m_active_entities.end(), vec.begin(), vec.end());
    return IteratorView{ m_active_entities.begin() + old_size, m_active_entities.end() };
}

template <typename EntityType>
void SceneOf<EntityType>::HomeSceneComplete::clear() {
    for (auto * cont : { &m_active_entities, &m_to_remove_entities, &m_active_entities }) {
        cont->clear();
    }
}

template <typename EntityType>
/* private */ void SceneOf<EntityType>::HomeSceneComplete::
    on_create(const EntityType & ent)
{ m_new_entities.push_back(ent); }

template <typename EntityType>
/* private */ void SceneOf<EntityType>::HomeSceneComplete::
    on_deletion_request(const EntityType & ent)
{
    if (!std::binary_search(m_active_entities.begin(), m_active_entities.end(), ent, compare_entities)) {
        throw InvArg("HomeScene::on_deletion_request: attempted to entity which does not belong to it's 'own' scene. (Home set wrong?)");
    }
    m_to_remove_entities.push_back(ent);
}

} // end of ecs namespace

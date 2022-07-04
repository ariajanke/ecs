#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/System/Sleep.hpp>

#include <common/sf/DrawText.hpp>
#include <common/sf/DrawLine.hpp>
#include <common/sf/DrawRectangle.hpp>
#include <common/sf/DrawTriangle.hpp>
#include <common/TypeList.hpp>
#include <common/Util.hpp>
#include <common/StringUtil.hpp>
#include <common/Vector2Util.hpp>
#include <common/BezierCurves.hpp>

#include <SFML/Graphics/Texture.hpp>

#include <random>
#include <iostream>

#include <cassert>

namespace {

using cul::DrawText;
using cul::DrawRectangle;
using cul::DrawTriangle;
using cul::BitmapFont;
using cul::DrawLine;
using cul::round_to;
using cul::SfBitmapFont;

constexpr const int k_window_width     = 800;
constexpr const int k_window_height    = 1000;
constexpr const int k_fps              =  60;
constexpr const int k_text_scale       =   2;
constexpr const int k_text_view_width  = k_window_width  / k_text_scale;
constexpr const int k_text_view_height = k_window_height / k_text_scale;

class AppState {
public:
    virtual ~AppState() {}
    virtual void setup() = 0;
    virtual void update(double) = 0;
    virtual void process_event(const sf::Event &) = 0;
    virtual void draw_to(sf::RenderTarget &) = 0;
    virtual const std::string & name() const = 0;

    std::unique_ptr<AppState> give_next_state() { return std::move(m_next_state); }

    static std::unique_ptr<AppState> default_instance();

protected:
    template <typename T>
    std::enable_if_t<std::is_base_of_v<AppState, T>, T &> set_next_state() {
        auto ptr = std::make_unique<T>();
        T * rv = ptr.get();
        m_next_state = std::move(ptr);
        return *rv;
    }

private:
    std::unique_ptr<AppState> m_next_state;
};

} // end of <anonymous> namespace

int main() {
#   if 0 // probably should not be in a demo app...
    SfBitmapFont::load_builtin_font(BitmapFont::k_8x8_font).texture().copyToImage().saveToFile("/media/ramdisk/cultextout.png");
    SfBitmapFont::load_builtin_font(BitmapFont::k_8x8_highlighted_font).texture().copyToImage().saveToFile("/media/ramdisk/cultextouthighlight.png");
    SfBitmapFont::load_builtin_font(BitmapFont::k_8x16_font).texture().copyToImage().saveToFile("/media/ramdisk/cultextout16.png");
    SfBitmapFont::load_builtin_font(BitmapFont::k_8x16_highlighted_font).texture().copyToImage().saveToFile("/media/ramdisk/cultextouthighlight16.png");
#   endif

    // test DrawRectangle's new constructor somewhere
    {
    [[maybe_unused]] DrawRectangle a(cul::Rectangle<int>{10, 30, 100, 80}, sf::Color(0x891932));
    }

    sf::RenderWindow win;
    win.setFramerateLimit(k_fps);
    win.create(sf::VideoMode(k_window_width, k_window_height), "Common Utilities Demo for SFML");
    auto app_state = AppState::default_instance();
    app_state->setup();

    DrawText title_text;
    title_text.load_builtin_font(BitmapFont::k_8x16_highlighted_font);
    title_text.set_text_top_left(sf::Vector2f(), app_state->name());

    while (win.isOpen()) {
        {
        sf::Event event;
        while (win.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                win.close();
            } else if (event.type == sf::Event::KeyReleased) {
                if (event.key.code == sf::Keyboard::Escape)
                    win.close();
            }
            app_state->process_event(event);
        }
        }
        if (auto new_state = app_state->give_next_state()) {
            app_state = std::move(new_state);
            app_state->setup();

            title_text.set_text_top_left(sf::Vector2f(), app_state->name());
        }
        app_state->update(1. / double(k_fps));

        win.clear(sf::Color(0, 80, 0));
        app_state->draw_to(win);
        auto states = sf::RenderStates::Default;
        states.transform.scale(k_text_scale, k_text_scale);
        win.draw(title_text, states);
        win.display();
        sf::sleep(sf::microseconds(1'000'000 / k_fps));
    }
    return 0;
}

namespace {

using Rng = std::default_random_engine;

using DoubleDistri = std::uniform_real_distribution<double>;
using FloatDistri  = std::uniform_real_distribution<float>;
using IntDistri    = std::uniform_int_distribution<int>;

class RandomRectangleState;
class RandomTriangleState;
class RandomTextState;
class LineTestState;
class BezierLineState;
class IntroductionState;

enum class FadeStage {
    k_fade_in, k_still, k_fade_out
};

constexpr const double k_fade_in_time  = 1.5;
constexpr const double k_still_time    = 3.5;
constexpr const double k_fade_out_time = 1. ;
constexpr const uint8_t k_u8_max = std::numeric_limits<uint8_t>::max();

inline uint8_t fade_in_alpha(double time_rem)
    { return round_to<uint8_t>((1. - (time_rem / k_fade_in_time))*k_u8_max); }

inline uint8_t fade_out_alpha(double time_rem)
    { return round_to<uint8_t>((time_rem / k_fade_out_time)*k_u8_max); }

inline bool check_and_progress(FadeStage & stage, double & time_rem, double et) {
    if ((time_rem -= et) > 0.) return false;
    switch (stage) {
    case FadeStage::k_fade_in:
        time_rem = k_still_time;
        stage    = FadeStage::k_still;
        break;
    case FadeStage::k_fade_out:
        time_rem = k_fade_in_time;
        stage    = FadeStage::k_fade_in;
        return true;
    case FadeStage::k_still:
        time_rem = k_fade_out_time;
        stage    = FadeStage::k_fade_out;
        break;
    }
    return false;
}

sf::Color random_color(Rng &);

class RotatingState : public AppState {
public:
    void process_event(const sf::Event &) override;

    using StateTypeList = cul::TypeList<
        RandomRectangleState, RandomTriangleState, RandomTextState,
        LineTestState, BezierLineState, IntroductionState>;

    template <typename T>
    static constexpr const int k_state_list_position_of
        = RotatingState::StateTypeList::GetTypeId<T>::k_value;

    virtual int position_in_state_list() const = 0;

private:
    template <typename ... Types>
    void switch_to_state_by_type_id(int, cul::TypeList<Types...>);

    template <typename Head, typename ... Types>
    void switch_to_state_by_type_id(int, cul::TypeList<Head, Types...>);
};

template <typename T>
class RotatingStateOf : public RotatingState {
    static_assert(StateTypeList::HasType<T>::k_value, "");

    int position_in_state_list() const final
        { return k_state_list_position_of<T>; }
};

class RandomRectangleState final : public RotatingStateOf<RandomRectangleState> {
public:
    void setup() final {
        m_drect          = random_rectangle(m_rng);
        m_stage          = FadeStage::k_fade_in;
        m_time_remaining = k_fade_in_time;
    }

    void update(double et) final {
        if (check_and_progress(m_stage, m_time_remaining, et)) {
            m_drect = random_rectangle(m_rng);
        }
        auto color = m_drect.color();
        switch (m_stage) {
        case FadeStage::k_fade_in:
            assert(color.a <= fade_in_alpha(m_time_remaining));
            color.a = fade_in_alpha(m_time_remaining);
            break;
        case FadeStage::k_fade_out:
            assert(color.a >= fade_out_alpha(m_time_remaining));
            color.a = fade_out_alpha(m_time_remaining);
            break;
        case FadeStage::k_still:
            color.a = 255;
            break;
        }
        m_drect.set_color(color);
    }

    void draw_to(sf::RenderTarget & target) final { target.draw(m_drect); }

    const std::string & name() const final {
        static const std::string k_name = "Random Rectangles";
        return k_name;
    }

private:
    // always alpha of 0
    static DrawRectangle random_rectangle(Rng & rng) {
        auto x = FloatDistri(0.f, k_window_width)(rng);
        auto y = FloatDistri(0.f, k_window_height)(rng);
        auto w = FloatDistri(0.f, k_window_width - x)(rng);
        auto h = FloatDistri(0.f, k_window_width - y)(rng);
        return DrawRectangle(x, y, w, h, random_color(rng));
    }

    DrawRectangle m_drect;

    double m_time_remaining = 0.;
    FadeStage m_stage;
    Rng m_rng { std::random_device()() };
};

class RandomTriangleState final : public RotatingStateOf<RandomTriangleState> {
public:
    void setup() final {
        m_dtri           = random_triangle(m_rng);
        m_stage          = FadeStage::k_fade_in;
        m_time_remaining = k_fade_in_time;
    }

    void update(double et) final {
        if (check_and_progress(m_stage, m_time_remaining, et)) {
            m_dtri = random_triangle(m_rng);
        }
        auto color = m_dtri.color();
        switch (m_stage) {
        case FadeStage::k_fade_in:
            color.a = fade_in_alpha(m_time_remaining);
            break;
        case FadeStage::k_fade_out:
            color.a = fade_out_alpha(m_time_remaining);
            break;
        case FadeStage::k_still:
            color.a = 255;
            break;
        }
        m_dtri.set_color(color);
    }

    void draw_to(sf::RenderTarget & target) final { target.draw(m_dtri); }

    const std::string & name() const final {
        static const std::string k_name = "Random Triangles";
        return k_name;
    }

private:
    static DrawTriangle random_triangle(Rng & rng) {
        std::array<sf::Vector2f, 3> pts;
        for (auto * fptr : { &pts[0].x, &pts[1].x, &pts[2].x }) {
            *fptr = FloatDistri(0.f, k_window_width)(rng);
        }
        for (auto * fptr : { &pts[0].y, &pts[1].y, &pts[2].y }) {
            *fptr = FloatDistri(0.f, k_window_height)(rng);
        }
        DrawTriangle dtri;
        dtri.set_point_a(pts[0]);
        dtri.set_point_b(pts[1]);
        dtri.set_point_c(pts[2]);
        dtri.set_color(random_color(rng));
        return dtri;
    }
    DrawTriangle m_dtri;
    double m_time_remaining = 0.;
    FadeStage m_stage;
    Rng m_rng { std::random_device()() };
};

class RandomTextState final : public RotatingStateOf<RandomTextState> {
public:
    void setup() final {
        static constexpr const int k_iters = 200;
        static constexpr const double k_total_setup_time = 5.;
        for (int i = 0; i != k_iters; ++i) {
            update(k_total_setup_time / k_iters);
        }
    }

    void update(double et) final {
        if (DoubleDistri(0., 1.)(m_rng) < probability_of_spawn()*et) {
            spawn_string<random_string>(random_screen_position(m_rng));
        }
        for (auto & text : m_texts) {
            text.location_y -= k_rise_speed*et;
            auto displacement = sf::Vector2f(0, -1.f)*k_rise_speed*float(et);
            for (auto & vtx : text.verticies) {
                vtx.position += displacement;
            }
        }
        auto tend = m_texts.end();
        m_texts.erase(std::remove_if(m_texts.begin(), tend, should_delete_float_text), tend);
    }

    void draw_to(sf::RenderTarget & target) final {
        auto states = sf::RenderStates::Default;
        states.transform.scale(k_text_scale, k_text_scale);
        for (auto & text : m_texts) {
            states.texture = text.texture;
            target.draw(text.verticies.data(), text.verticies.size(),
                        DrawText::k_primitive_type, states);
        }
    }

    void process_event(const sf::Event & event) final {
        RotatingState::process_event(event);
        if (event.type == sf::Event::MouseButtonReleased) {
            spawn_string<random_click_string>(
                sf::Vector2f(event.mouseButton.x / k_text_scale,
                             event.mouseButton.y / k_text_scale));
        }
    }

    const std::string & name() const final {
        static const std::string k_name = "Random Text";
        return k_name;
    }

private:
    static constexpr float k_rise_speed = 75.f;

    double probability_of_spawn() const {
        // probability of spawn per second
        static constexpr const double k_min_probability = 0.05;
        static constexpr const double k_max_probability = 1.75;
        static constexpr const int k_quantity_to_min = 30;
        if (m_texts.size() > k_quantity_to_min) return k_min_probability;
        auto intpl = 1. - (double(m_texts.size()) / double(k_quantity_to_min));
        return k_min_probability + (k_max_probability - k_min_probability)*intpl;
    }

    static sf::Vector2f random_screen_position(Rng & rng) {
        return sf::Vector2f(FloatDistri(0.f, k_text_view_width )(rng),
                            FloatDistri(0.f, k_text_view_height)(rng));
    }

    template <const std::string & (*choose_string_f)(Rng &)>
    void spawn_string(sf::Vector2f position) {
        DrawText dtext;
        static const std::array k_font_choices {
            BitmapFont::k_8x8_font, BitmapFont::k_8x8_highlighted_font,
            BitmapFont::k_8x16_font, BitmapFont::k_8x16_highlighted_font
        };
        dtext.load_builtin_font(k_font_choices[ IntDistri(0, k_font_choices.size() - 1)(m_rng) ]);
        dtext.set_text_center(position, choose_string_f(m_rng));

        m_texts.emplace_back();
        m_texts.back().location_y  = k_window_height + dtext.font()->character_size().height / 2;
        m_texts.back().char_height = dtext.font()->character_size().height;
        m_texts.back().texture     = &dtext.font()->texture();
        m_texts.back().verticies   = dtext.give_verticies();
    }

    static const std::string & random_string(Rng & rng) {
        using Str = std::string;
        static std::array strings {
            Str("Hello There"),
            Str("This is a test string"),
            Str("Click somewhere!"),
            Str("There was quick brown fox."),
            Str("That jumps over the lazy dog.")
        };
        return strings[IntDistri(0, strings.size() - 1)(rng)];
    }

    static const std::string & random_click_string(Rng & rng) {
        using Str = std::string;
        static std::array strings {
            Str("Click!"),
        };
        return strings[IntDistri(0, strings.size() - 1)(rng)];
    }

    struct FloatText {
        const sf::Texture * texture = nullptr;
        float location_y = 0.f;
        float char_height = 0.f;
        std::vector<sf::Vertex> verticies;
    };

    static bool should_delete_float_text(const FloatText & text)
        { return text.location_y + text.char_height * 0.5f < 0.f; }

    std::vector<FloatText> m_texts;
    Rng m_rng { std::random_device()() };
};

class LineTestState final : public RotatingStateOf<LineTestState> {
public:
    void setup() final {
        update_lines();
    }

    void update(double et) final {
        m_elapsed_time += et;
        update_lines();
    }

    void draw_to(sf::RenderTarget & target) final {
        target.draw(m_verticies.data(), m_verticies.size(), sf::PrimitiveType::Triangles);
        for (const auto & line : m_lines) {
            target.draw(line);
        }
    }

    const std::string & name() const final {
        static const std::string k_name = "Line Rendering Tests";
        return k_name;
    }

private:
    void update_lines() {
        static constexpr const double k_pi = cul::k_pi_for_type<double>;
        sf::Vector2f center(k_window_width / 2, k_window_height / 2);
        m_verticies.clear();
        m_verticies.reserve(k_line_count*6);
        m_lines.clear();
        m_lines.reserve(k_line_count);
        auto t_offset = std::fmod(m_elapsed_time*k_rotation_speed, 2.*k_pi);
        auto thickness = k_thickness_minimum
            +  (std::sin( m_elapsed_time*k_thickness_change_speed ) + 1.f)
              * 0.5f * (k_thickness_maximum - k_thickness_minimum);
        for (int i = 0; i != k_line_count / 2; ++i) {
            auto t = (double(i) / double(k_line_count))*2.*k_pi + t_offset;
            auto pt = sf::Vector2f(std::cos(t), std::sin(t))*0.8f*float(std::min(k_window_width / 2, k_window_height / 2));
            DrawLine dline(center, center + pt, thickness, sf::Color::White);
            m_lines.emplace_back(dline);
        }
    }

    // radians per second
    static const constexpr double k_rotation_speed         = 0.25;
    // also radian per second (sine between [0 1])
    static const constexpr double k_thickness_change_speed = 0.3;

    static const constexpr double k_thickness_maximum      = 15.;
    static const constexpr double k_thickness_minimum      =  1.;

    static const constexpr int k_line_count = 28;

    std::vector<sf::Vertex> m_verticies;
    std::vector<DrawLine> m_lines;

    double m_elapsed_time = 0.;
};

class BezierLineState final : public RotatingStateOf<BezierLineState> {
public:
    using VecF = cul::Vector2<float>;

    struct PointParameters {
        VecF anchor = VecF(0.5*k_window_width, k_window_height*0.75f);
        // default location of the tip without any mouse movement
        VecF default_tip_location = VecF(0.5*k_window_width, 0.2f*k_window_height);
        // distance from anchor to an anchor end point
        float distance_to_anchor_end     = k_window_height / 10.f;
        // distance from an anchor end point to control point
        float distance_to_anchor_control = k_window_width  / 8.f;

        float distance_to_tip_control = k_window_height / 5.f;
    };

    void setup() final {
        const PointParameters p;
        m_tip = p.default_tip_location;
        m_verticies = make_verticies(m_tip);
    }

    void update(double) final {}

    void draw_to(sf::RenderTarget & target) final {
        target.draw(m_verticies.data(), m_verticies.size(), sf::PrimitiveType::Triangles);

        auto tuples = get_bezier_tuples(m_tip);
        cul::for_bezier_lines(tuples.left_points, 10, [&target](VecF a, VecF b) {
            using cul::convert_to;
            DrawLine dline(convert_to<sf::Vector2f>(a), convert_to<sf::Vector2f>(b), 3.f, sf::Color::Red);
            target.draw(dline);
        });
        cul::for_bezier_lines(tuples.right_points, 10, [&target](VecF a, VecF b) {
            using cul::convert_to;
            DrawLine dline(convert_to<sf::Vector2f>(a), convert_to<sf::Vector2f>(b), 3.f, sf::Color::Blue);
            target.draw(dline);
        });
    }

    const std::string & name() const final {
        static const std::string k_name = "Bezier Lines";
        return k_name;
    }

    void process_event(const sf::Event & event) final {
        RotatingState::process_event(event);
        if (event.type == sf::Event::MouseMoved) {
            m_tip.x = float(event.mouseMove.x);
            m_tip.y = float(event.mouseMove.y);

            m_verticies = make_verticies(m_tip, std::move(m_verticies));
        } else if (   event.type == sf::Event::MouseLeft
                   || event.type == sf::Event::LostFocus)
        {
            m_tip = PointParameters().default_tip_location;
            m_verticies = make_verticies(m_tip, std::move(m_verticies));
        }
    }

private:
    static constexpr const float k_pi_f  = cul::k_pi_for_type<float>;
    // lets think long and hard what exactly direction means here...
    static constexpr const float k_left  = -1.f;
    static constexpr const float k_right =  1.f;

    struct TuplePtPair {
        // tip, tip control, anchor control, anchor end
        std::tuple<VecF, VecF, VecF, VecF> left_points ;
        std::tuple<VecF, VecF, VecF, VecF> right_points;
    };

    static VecF to_anchor_end(const PointParameters & p, float dir_) {
        assert(dir_ == k_left || dir_ == k_right);
        return VecF(dir_, 0.f)*p.distance_to_anchor_end;
    }

    static VecF get_anchor_control
        (const PointParameters & p, float amount, float dir_)
    {
        assert(dir_ == k_left || dir_ == k_right);
        using namespace cul;
        // to control without angle
        const auto v_to_anchor_end = to_anchor_end(p, dir_);
        const auto theta_0 = angle_between(
            VecF(-dir_, 0.f), // toward the anchor
            p.default_tip_location - (p.anchor + v_to_anchor_end));
        return p.anchor + v_to_anchor_end + rotate_vector(
            VecF(-dir_, 0.f)*p.distance_to_anchor_control,
            dir_*(theta_0 + (k_pi_f - theta_0)*amount));
    }

    static VecF get_tip_control
        (const PointParameters & p, VecF tip, float amount, float dir_)
    {
        assert(dir_ == k_left || dir_ == k_right);
        using namespace cul;

        const auto theta_0 = directed_angle_between(
            p.anchor + to_anchor_end(p, dir_) - p.default_tip_location,
            p.anchor                          - p.default_tip_location);
        auto theta = normalize(theta_0)*(k_pi_f*0.5f - magnitude(theta_0))*amount;
        // must be the same sign
        assert(theta*theta_0 >= 0.f);
        return tip + rotate_vector(normalize(p.anchor - tip)*p.distance_to_tip_control,
                                   theta_0 + theta);
    }

    static TuplePtPair get_bezier_tuples(VecF tip) {
        using namespace cul;
        using std::make_tuple;
        static const PointParameters p;
        VecF anchor_left  = p.anchor + to_anchor_end(p, k_left );
        VecF anchor_right = p.anchor + to_anchor_end(p, k_right);
        float squish_amount = std::max(0.f, 1.f - magnitude(tip - p.anchor) / magnitude(p.default_tip_location - p.anchor));
        TuplePtPair rv;
        rv.left_points = make_tuple(tip,
            get_tip_control(p, tip, squish_amount, k_left),
            get_anchor_control(p, squish_amount, k_left), anchor_left);
        rv.right_points = make_tuple(tip,
            get_tip_control(p, tip, squish_amount, k_right),
            get_anchor_control(p, squish_amount, k_right), anchor_right);
        return rv;
    }

    static std::vector<sf::Vertex> make_verticies
        (VecF tip, std::vector<sf::Vertex> && rv = std::vector<sf::Vertex>())
    {
        auto tuples = get_bezier_tuples(tip);
        rv.clear();
        for_bezier_triangles(tuples.left_points, tuples.right_points,
                       25.f*25.f, 0.5f, [&rv](VecF a, VecF b, VecF c)
        {
            using namespace cul;
            rv.emplace_back(convert_to<sf::Vector2f>(a));
            rv.emplace_back(convert_to<sf::Vector2f>(b));
            rv.emplace_back(convert_to<sf::Vector2f>(c));
        });
        return std::move(rv);
    }

    template <typename T>
    static T max_length_from_area(T resolution_area) {
        return std::sqrt(resolution_area * T(4) / std::sqrt(T(3)) );
    }

    VecF m_tip;
    std::vector<sf::Vertex> m_verticies;
};

inline bool is_newline(char c) { return c == '\n'; }

template <typename IterType, typename HandleSequenceFunc>
void wrap_and_split_nl(IterType beg, IterType end, int width_in_chars, HandleSequenceFunc && f) {
    cul::wrap_string_as_monowidth(beg, end, width_in_chars,
        [&f](const char * beg, const char * end)
    { cul::for_split<is_newline>(beg, end, f); });
}

class IntroductionState final : public RotatingStateOf<IntroductionState> {
public:
    static constexpr const auto k_intro_font = BitmapFont::k_8x16_highlighted_font;

    void setup() final {
        using namespace cul;
        const auto & intro_font_inst = SfBitmapFont::load_builtin_font(k_intro_font);
        int view_width_in_chars = k_text_view_width / intro_font_inst.character_size().width;
        int line_count = 0;
        wrap_and_split_nl(k_intro_text, k_intro_text_end, view_width_in_chars,
                          [&line_count](const char *, const char *) { ++line_count; });

        auto y_start = (k_text_view_height - intro_font_inst.character_size().height*line_count ) / 2;
        m_texts.reserve(line_count);
        wrap_and_split_nl(k_intro_text, k_intro_text_end, view_width_in_chars,
            [this, &y_start, &intro_font_inst](const char * beg, const char * end)
        {
            DrawText dtext;
            dtext.assign_font(intro_font_inst);
            dtext.set_text_center(sf::Vector2f(k_text_view_width / 2, y_start),
                                  beg, end);
            m_texts.emplace_back(std::move(dtext));
            y_start += intro_font_inst.character_size().height;
        });
    }

    void update(double) final {}

    void draw_to(sf::RenderTarget & target) final {
        for (const auto & text : m_texts) {
            auto states = sf::RenderStates::Default;
            states.transform.scale(k_text_scale, k_text_scale);
            target.draw(text, states);
        }
    }

    const std::string & name() const final {
        static const std::string k_name = "Introduction Screen";
        return k_name;
    }

private:
    static constexpr const char * k_intro_text =
        "Hello, this is my Utilities for SFML Demo App.\n"
        "The current state's name is in the top left corner. "
        "Left and right arrow keys will switch app states.\n"
        "Lastly: press Escape to quit.";

    static constexpr const char * k_intro_text_end =
        cul::find_str_end(k_intro_text);

    std::vector<DrawText> m_texts;
};

sf::Color random_color(Rng & rng) {
    using Uint8Distri = std::uniform_int_distribution<uint8_t>;
    std::array<uint8_t, 3> attrs;
    attrs[0] = Uint8Distri(180, 255)(rng);
    attrs[1] = (DoubleDistri(0., 1.)(rng) > 0.5 ? Uint8Distri(0, 100) : Uint8Distri(180, 255))(rng);
    attrs[2] = (DoubleDistri(0., 1.)(rng) > 0.25 ? Uint8Distri(0, 100) : Uint8Distri(180, 255))(rng);
    std::shuffle(attrs.begin(), attrs.end(), rng);
    return sf::Color(attrs[0], attrs[1], attrs[2], 0);
}

/* static */ std::unique_ptr<AppState> AppState::default_instance() {
    return std::make_unique<IntroductionState>();
}

void RotatingState::process_event(const sf::Event & event) {
    if (event.type != sf::Event::KeyReleased) return;
    static constexpr const auto k_state_count = StateTypeList::k_count;
    switch (event.key.code) {
    case sf::Keyboard::Left: {
        auto np = position_in_state_list() - 1;
        if (np == -1) np = k_state_count - 1;
        switch_to_state_by_type_id(np, StateTypeList());
        }
        break;
    case sf::Keyboard::Right: {
        auto np = position_in_state_list() + 1;
        switch_to_state_by_type_id(np % k_state_count, StateTypeList());
        }
        break;
    default: break;
    }
}

template <typename ... Types>
/* private */ void RotatingState::switch_to_state_by_type_id(int, cul::TypeList<Types...>) {
    throw std::invalid_argument("index not a valid type id");
}

template <typename Head, typename ... Types>
/* private */ void RotatingState::switch_to_state_by_type_id(int idx, cul::TypeList<Head, Types...>) {
    if (idx == k_state_list_position_of<Head>) {
        return (void)set_next_state<Head>();
    }
    switch_to_state_by_type_id(idx, cul::TypeList<Types...>());
}

} // end of <anonymous> namespace

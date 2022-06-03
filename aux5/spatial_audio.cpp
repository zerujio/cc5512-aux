/*
 * Este ejemplo está basado en el auxiliar 4.
 * Los contenidos de ese ejemplo fueron movidos a engine[.cpp|.hpp]
 *
 * Audio: https://freesound.org/people/BaDoink/sounds/537020/
 */

#include "engine.hpp"

#define DR_WAV_IMPLEMENTATION
#include <dr_wav.h>

#include <AL/al.h>

#include <iostream>

// Checkeo de errores para las llamadas de OpenAL
template<auto ALFunction, class... Args>
void ALCall(Args... args) {
    alGetError();
    ALFunction(args...);
    auto error = alGetError();
    if (error) {
        switch (error) {
#define CASE(X) case X: throw std::runtime_error("[OpenAL Error] " #X);
            CASE(AL_INVALID_NAME)
            CASE(AL_INVALID_ENUM)
            CASE(AL_INVALID_VALUE)
            CASE(AL_INVALID_OPERATION)
            CASE(AL_OUT_OF_MEMORY)
            default:
                throw std::runtime_error("Unknown OpenAL error");
        }
    }
}

template <auto alGen, auto alDelete>
class ALObject
{
public:
    ALObject() {
        ALCall<alGen>(1, &m_id);
    }
    ~ALObject() {
        deleteObject();
    }
    ALObject(ALObject&& o) noexcept {
        deleteObject();
        m_id = o.m_id;
        o.m_id = 0;
    }
    ALObject& operator= (ALObject&& o) noexcept {
        deleteObject();
        m_id = o.m_id;
        o.m_id = 0;
        return *this;
    }
protected:
    ALuint m_id {0};

private:
    void deleteObject() noexcept
    try {
        ALCall<alDelete>(1, &m_id);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
};

class RAudioBuffer final : public ALObject<alGenBuffers, alDeleteBuffers> {
    friend class CAudioSource;
public:
    enum class Format {
        Mono8 = AL_FORMAT_MONO8,
        Mono16 = AL_FORMAT_MONO16,
        Stereo8 = AL_FORMAT_STEREO8,
        Stereo16 = AL_FORMAT_STEREO16
    };

    void setData(Format fmt, int freq, int size, const void* data) const {
        ALCall<alBufferData>(m_id, static_cast<ALenum>(fmt), data, size, freq);
    }
};

class CAudioSource final : public ALObject<alGenSources, alDeleteSources> {
public:
    void play() const { ALCall<alSourcePlay>(m_id); }
    void pause() const { ALCall<alSourcePause>(m_id); }
    void stop() const { ALCall<alSourceStop>(m_id); }
    void rewind() const { ALCall<alSourceRewind>(m_id); }

    void setLooping(bool b) const {
        ALCall<alSourcei>(m_id, AL_LOOPING, b);
    }

    [[nodiscard]]
    bool getLooping() const {
        int b;
        ALCall<alGetSourcei>(m_id, AL_LOOPING, &b);
        return b;
    }

    void setBuffer(std::shared_ptr<RAudioBuffer> buffer) {
        ALCall<alSourcei>(m_id, AL_BUFFER, buffer->m_id);
        m_buffer = std::move(buffer);
    }

    [[nodiscard]]
    std::shared_ptr<RAudioBuffer> getBuffer() const {
        return m_buffer;
    }

    void setRolloffFactor(float factor) const {
        ALCall<alSourcef>(m_id, AL_ROLLOFF_FACTOR, factor);
    }

    friend void updateSource(const CAudioSource&, const CTransform&, double delta);

private:
    std::shared_ptr<RAudioBuffer> m_buffer;

};

std::shared_ptr<RAudioBuffer> loadAudioClip(const char* file_path) {
    unsigned int channels;
    unsigned int sample_rate;
    drwav_uint64 frame_count;
    drwav_int16 *data = drwav_open_file_and_read_pcm_frames_s16(
            file_path,
            &channels,
            &sample_rate,
            &frame_count,
            nullptr);

    if (!data) {
        throw std::runtime_error(std::string("Failed to open file: ") + file_path);
    }

    using Format = RAudioBuffer::Format;
    const auto fmt = channels == 1 ? Format::Mono16 : Format::Stereo16;
    const auto size = sizeof(drwav_int16) * frame_count;
    auto buffer = std::make_shared<RAudioBuffer>();
    buffer->setData(fmt, sample_rate, size, data);

    drwav_free(data, nullptr);

    return buffer;
}

// Invocada cuando comienza la escena.
void init(Scene &scene) {
    auto program = std::make_shared<RProgram>(makeProgram("vert.glsl", "frag.glsl"));

    const auto mesh_data = createCubeMesh();
    auto mesh = std::make_shared<RMesh>(mesh_data.vao, mesh_data.ebo, mesh_data.index_count);

    scene.root.entity = scene.player;
    scene.registry.emplace<CTransform>(scene.player);
    scene.registry.emplace<CVisual>(scene.player, glm::vec4(.25, .5, 1., 1.), mesh, program);

    auto obj1 = scene.registry.create();
    scene.root.children.emplace_front(obj1);
    scene.registry.emplace<CTransform>(obj1, glm::vec3(0., 0., 3.));
    scene.registry.emplace<CVisual>(obj1, glm::vec4(1., .5, 0., 1.), mesh, program);

    auto obj2 = scene.registry.create();
    scene.root.children.front().children.emplace_front(obj2);
    scene.registry.emplace<CTransform>(obj2, glm::vec3(0., 0., 3.));
    scene.registry.emplace<CVisual>(obj2, glm::vec4(1., 0., 0., 1.), mesh, program);

    auto audio_buffer = loadAudioClip("loop.wav");
    auto& src = scene.registry.emplace<CAudioSource>(obj2);
    src.setBuffer(audio_buffer);
    src.setLooping(true);
    src.setRolloffFactor(2.5f);
    src.play();

    scene.camera.eye = {0, 1, 5};
}

void updateListener(const Camera& camera, double delta) {
    glm::vec3 last_pos;
    ALCall<alGetListenerfv>(AL_POSITION, glm::value_ptr(last_pos));

    ALCall<alListenerfv>(AL_POSITION, glm::value_ptr(camera.eye));

    glm::vec3 velocity = (camera.eye - last_pos) / float(delta);
    ALCall<alListenerfv>(AL_VELOCITY, glm::value_ptr(velocity));

    glm::vec3 orientation[] {camera.at - camera.eye, camera.up};
    ALCall<alListenerfv>(AL_ORIENTATION, glm::value_ptr(orientation[0]));
}

std::ostream& operator<< (std::ostream& os, glm::vec3& v) {
    return os << "{" << v.x << ", " << v.y << ", " << v.z << "}";
}

void updateSource(const CAudioSource& src, const CTransform& tr, double delta) {
    glm::vec3 last_pos;
    ALCall<alGetSourcefv>(src.m_id, AL_POSITION, glm::value_ptr(last_pos));

    glm::vec3 curr_pos = tr.matrix * glm::vec4(0, 0, 0, 1); // <- aquí se calcula la posición en coordenadas globales.
    ALCall<alSourcefv>(src.m_id, AL_POSITION, glm::value_ptr(curr_pos));

    glm::vec3 velocity = (curr_pos - last_pos) / float(delta);
    ALCall<alSourcefv>(src.m_id, AL_VELOCITY, glm::value_ptr(velocity));
}

// En cada cuadro
void update(Scene &scene, double delta) {
    static float alpha = 0.0f;
    constexpr float ang_spd = .76f;
    alpha += ang_spd * delta;
    scene.registry.get<CTransform>(scene.player).position = {0., 1 * std::sin(alpha), 0.};
    scene.registry.get<CTransform>(scene.player).rotation = {0., alpha, 0.};

    updateListener(scene.camera, delta);
    scene.registry.view<CAudioSource, CTransform>().each(
        [delta](const CAudioSource& s, const CTransform& t) { updateSource(s, t, delta); }
    );
}

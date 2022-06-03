#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include "AL/al.h"
#include "AL/alc.h"

#include <memory>

#include <entt/entt.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>


struct Transform {
    glm::vec3 position;
};

struct Camera {
    glm::vec3 eye;
    glm::vec3 at;
    glm::vec3 up;
};

class AudioBuffer {
    friend class CAudioSource;
public:
    AudioBuffer() {
        alGenBuffers(1, &m_id);
    }
    ~AudioBuffer() {
        alDeleteBuffers(1, &m_id);
    }
    AudioBuffer(AudioBuffer&& other) noexcept
    : m_id(other.m_id) {
        other.m_id = 0;
    }
    AudioBuffer& operator=(AudioBuffer&& other)  noexcept {
        m_id = other.m_id;
        other.m_id = 0;
    }

    enum class Format {
        Mono8 = AL_FORMAT_MONO8,
        Mono16 = AL_FORMAT_MONO16,
        Stereo8 = AL_FORMAT_STEREO8,
        Stereo16 = AL_FORMAT_STEREO16
    };

    void setData(Format fmt, int freq, int size, const void* data) const {
        alBufferData(m_id, static_cast<ALenum>(fmt), data, size, freq);
    }

private:
    ALuint m_id;
};

class CAudioSource {
public:
    CAudioSource() {
        alGenSources(1, &m_id);
    }

    ~CAudioSource() {
        alDeleteSources(1, &m_id);
    }

    CAudioSource(CAudioSource&& other) noexcept
    : m_id(other.m_id)
    {
        other.m_id = 0;
    }

    CAudioSource& operator=(CAudioSource&& other)  noexcept {
        m_id = other.m_id;
        other.m_id = 0;
    }

    void play() const { alSourcePlay(m_id); }
    void stop() const { alSourceStop(m_id); }
    void pause() const { alSourcePause(m_id); }
    void rewind() const { alSourceRewind(m_id); }

    void setBuffer(std::shared_ptr<AudioBuffer> buf) {
        m_buf = std::move(buf);
        alSourcei(m_id, AL_BUFFER, m_buf->m_id);
    }

    [[nodiscard]]
    std::shared_ptr<AudioBuffer> getBuffer() const { return m_buf; }

    static void updateSource(const Transform& tr, const CAudioSource& src, float delta) {
        glm::vec3 last_pos;
        alGetSourcefv(src.m_id, AL_POSITION, glm::value_ptr(last_pos));

        alSourcefv(src.m_id, AL_POSITION, glm::value_ptr(tr.position));

        glm::vec3 velocity = (tr.position - last_pos) / delta;
        alSourcefv(src.m_id, AL_VELOCITY, glm::value_ptr(velocity));
    }

private:
    ALuint m_id;
    std::shared_ptr<AudioBuffer> m_buf;
};

std::shared_ptr<AudioBuffer> loadAudio(const char* file_path);


int main() {

    // inicialización
    // crear contexto de OpenGL, OpenAL, ventana, etc

    auto a_buffer0 = loadAudio("holi.wav");
    auto a_buffer1 = loadAudio("adios.wav");

    entt::registry registry;

    auto entity0 = registry.create();
    auto entity1 = registry.create();

    registry.emplace<Transform>(entity0);
    registry.emplace<Transform>(entity1);

    auto& src0 = registry.emplace<CAudioSource>(entity0);
    src0.setBuffer(a_buffer0);

    auto& src1 = registry.emplace<CAudioSource>(entity1);
    src1.setBuffer(a_buffer1);

    src0.play();
    src1.play();

    Camera camera;

    float delta = 0.016;
    while (true) {

        // update listener
        glm::vec3 last_pos;
        alGetListenerfv(AL_POSITION, glm::value_ptr(last_pos));

        alListenerfv(AL_POSITION, glm::value_ptr(camera.eye));

        glm::vec3 velocity = (camera.eye - last_pos) / delta;
        alListenerfv(AL_VELOCITY, glm::value_ptr(velocity));

        glm::vec3 orientation[] {camera.at, camera.up};
        alListenerfv(AL_ORIENTATION, glm::value_ptr(orientation[0]));

        registry.view<Transform, CAudioSource>().each(
                [delta](const Transform& tr, const CAudioSource& src)
                { CAudioSource::updateSource(tr, src, delta); }
            );
    }
}
#include "impl/app/InternalSystems.h"

void TransformUpdate::Update()
{
	for (auto [transform] : Query<Transform2D>())
	{
		transform.UpdateLastFrameData();
	}
}

void PhysicsInterpolationUpdate::Update()
{
	m_acc += Time::DeltaTime();
	float ratio = clamp(m_acc / Time::RawFixedTime(), 0.f, 1.f);

	for (auto [transform, body] : Query<Transform2D, Rigidbody2D>())
	{
		if (!body.InWorld()) continue;

		transform.position.x = lerp(body.LastTransform.position.x, body.GetPosition().x, ratio);
		transform.position.y = lerp(body.LastTransform.position.y, body.GetPosition().y, ratio);
		transform.rotation   = lerp(body.LastTransform.rotation,   body.GetAngle(),      ratio);
	}
}

void PhysicsInterpolationUpdate::FixedUpdate()
{
	m_acc = 0;

	for (auto [transform, body] : Query<Transform2D, Rigidbody2D>())
	{
		if (!body.InWorld()) continue;

		body.LastTransform.position = body.GetPosition();
		body.LastTransform.rotation = body.GetAngle();
	}
}

void AudioUpdate::LoadBank(AudioBankLoader& loader)
{
	for (const std::string& bank : loader.banks)
	{
		m_audio->Load(bank);
	}
}

void AudioUpdate::FreeBank(AudioBankLoader& loader)
{
	for (const std::string& bank : loader.banks)
	{
		m_audio->Free(m_audio->GetHandle(bank));
	}
}

void AudioUpdate::Init()
{
	m_audio = &GetWorld()->GetApplication()->GetAudio();

	for (auto [loader] : Query<AudioBankLoader>())
	if (loader.load == AudioBankLoader::ON_INIT)
		LoadBank(loader);
}

void AudioUpdate::Dnit()
{
	for (auto [loader] : Query<AudioBankLoader>())
	if (loader.load == AudioBankLoader::ON_DNIT)
		FreeBank(loader);
}

void AudioUpdate::Activate()
{
	for (auto [loader] : Query<AudioBankLoader>())
	if (loader.load == AudioBankLoader::ON_ACTIVE)
		LoadBank(loader);
}

void AudioUpdate::Deactivate()
{
	for (auto [loader] : Query<AudioBankLoader>())
	if (loader.load == AudioBankLoader::ON_DEACTIVE)
		FreeBank(loader);
}

void AudioUpdate::Update()
{
	//Audio& audio = GetWorld()->GetApplication()->GetAudio();

	//for (auto [emitter] : Query<AudioEmitter>())
	//{
	//	if (!emitter.IsPlaying())
	//	{
	//		emitter.instance = audio.Play(emitter.instance);
	//	}

	//	GetWorld()->GetAudioWorld().Load();
	//}
}

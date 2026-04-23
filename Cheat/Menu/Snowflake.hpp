#pragma once

namespace Snowflake
{
	struct SnowF
	{
		ImVec2 Pos;
		float snowSpeed, snowMotion, Size;
		ImU32 Color;
	};

	void Snowflakes()
	{
		static std::vector<SnowF> Snow(100);
		static std::vector<SnowF> Snow1(100);
		static bool Init = false;
		auto Screen = ImGui::GetIO().DisplaySize;

		if (!Init)
		{
			for (auto& S : Snow)
			{
				S.Pos = ImVec2(rand() % (int)Screen.x, rand() % (int)Screen.y);
				S.snowMotion = 1.5f + (rand() % 100) / 200.0f;
				S.Size = 1.5f + rand() % 2;

				if (rand() % 100 < 30)
				{
					int gray = 20 + rand() % 30;
					S.Color = IM_COL32(gray, gray, gray, 100);
					S.snowSpeed = 0.2f + (rand() % 100) / 300.0f;
				}
				else if (rand() % 100 < 10)
				{
					int red = 0;
					int green = 0;
					int blue = 200 + rand() % 55;
					int alpha = 100;
					S.Color = IM_COL32(red, green, blue, alpha);
					S.snowSpeed = 0.3f + (rand() % 100) / 200.0f;
				}
				else
				{
					int red = 0;
					int green = 0;
					int blue = 50 + rand() % 100;
					int alpha = 100;
					S.Color = IM_COL32(red, green, blue, alpha);
					S.snowSpeed = 0.3f + (rand() % 100) / 200.0f;
				}
			}

			for (auto& S : Snow1)
			{
				S.Pos = ImVec2(rand() % (int)Screen.x, rand() % (int)Screen.y);
				S.snowMotion = 1.f + (rand() % 100) / 200.0f;
				S.Size = 1.5f + rand() % 2;

				if (rand() % 100 < 30)
				{
					int gray = 20 + rand() % 30;
					S.Color = IM_COL32(gray, gray, gray, 255);
					S.snowSpeed = 0.1f + (rand() % 100) / 300.0f;
				}
				else if (rand() % 100 < 10)
				{
					int red = 0;
					int green = 0;
					int blue = 200 + rand() % 55;
					int alpha = 255;
					S.Color = IM_COL32(red, green, blue, alpha);
					S.snowSpeed = 0.2f + (rand() % 100) / 200.0f;
				}
				else
				{
					int red = 0;
					int green = 0;
					int blue = 50 + rand() % 100;
					int alpha = 255;
					S.Color = IM_COL32(red, green, blue, alpha);
					S.snowSpeed = 0.2f + (rand() % 100) / 200.0f;
				}
			}

			Init = true;
		}

		static float time = 0.0f;
		time += ImGui::GetIO().DeltaTime;

		for (auto& S : Snow)
		{
			S.Pos.y += S.snowSpeed;
			S.Pos.x += sin(time * S.snowMotion) * 0.5f;

			if (S.Pos.y > Screen.y)
			{
				S.Pos.y = 0;
				S.Pos.x = rand() % (int)Screen.x;
			}


			ImU32 glowColor = IM_COL32(
				(S.Color >> 24) & 0xFF,
				(S.Color >> 16) & 0xFF,
				(S.Color >> 8) & 0xFF,
				(S.Color & 0xFF) * 0.3f
			);


			ImGui::GetBackgroundDrawList()->AddCircleFilled(S.Pos, S.Size * 1.5f, glowColor);

			ImGui::GetBackgroundDrawList()->AddCircleFilled(S.Pos, S.Size, S.Color);
		}

		for (auto& S : Snow1)
		{
			S.Pos.y += S.snowSpeed;
			S.Pos.x += sin(time * S.snowMotion) * 1.f;

			if (S.Pos.y > Screen.y)
			{
				S.Pos.y = 0;
				S.Pos.x = rand() % (int)Screen.x;
			}


			ImU32 glowColor = IM_COL32(
				(S.Color >> 24) & 0xFF,
				(S.Color >> 16) & 0xFF,
				(S.Color >> 8) & 0xFF,
				(S.Color & 0xFF) * 0.3f
			);


			ImGui::GetForegroundDrawList()->AddCircleFilled(S.Pos, S.Size * 1.5f, glowColor);

			ImGui::GetForegroundDrawList()->AddCircleFilled(S.Pos, S.Size, S.Color);
		}
	}
}
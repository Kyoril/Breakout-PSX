/*
 * Breakanoid
 * Copyright (C) 2022, Kyoril. All rights reserved.
 * ================================================
 * This file contains function implementations for 
 * the title state, which is the intial game state.
 */

#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>

#include "Title.h"
#include "Engine.h"
#include "Breakout.h"

int HandleGsTitle()
{
	int i;
	GsIMAGE titleImage;
	GsSPRITE titleSprite;
	u_char direction = 0;
	u_char textColor = 128;
	int selection = 0;
	u_char startPressed = 0;
	ControllerPacket* controllerPacket = 0;

	u_char downPressed = 0;
	u_char upPressed = 0;
	u_char leftAxis = 128;
	u_char tmpAxis = 128;

	int menuItemCount = 2;
	char* menuItems[] =
	{
		"Start Demo",
		"Options"
	};

	if (!LoadTIMFile("TITLE.TIM", &titleImage))
	{
		ErrorMessage("TITLE.TIM not found");
	}

	titleSprite = CreateSprite(titleImage, 0, 0, 256, 64, 0, 0);
	titleSprite.x = 24;

	while(1)
	{
		BeginFrame();

		if (direction == 0)
		{
			if (textColor >= 253)
			{
				direction = 1;
			}
			else 
			{
				textColor += 2;
			}
		}
		else
		{
			if (textColor < 2)
			{
				direction = 0;
			}
			else 
			{
				textColor -= 2;
			}
		}

		controllerPacket = GetControllerPacket(0);

		if (!startPressed)
		{
			DrawTextColored("Press START!", 110, 148, textColor, textColor, textColor);

			if (ControllerPacketIsValid(controllerPacket))
			{
				if (IsPadButtonPressed(controllerPacket, PAD_Start))
				{
					startPressed = 1;
				}
			}
		}
		else
		{
			if (ControllerPacketIsValid(controllerPacket))
			{
				if (IsPadButtonPressed(controllerPacket, PAD_Select))
				{
					startPressed = 0;
				}
				else
				{
					if (IsPadButtonPressed(controllerPacket, PAD_Down))
					{
						if (!downPressed)
						{
							selection = (selection + 1) % menuItemCount;
						}
						downPressed = 1;
					}
					else
					{
						downPressed = 0;
					}
				}
				
				if (IsPadButtonPressed(controllerPacket, PAD_Up))
				{
					if (!upPressed)
					{
						selection = (selection + menuItemCount - 1) % menuItemCount;
					}
					upPressed = 1;
				}
				else
				{
					upPressed = 0;
				}

				if (IsPadButtonPressed(controllerPacket, PAD_Cross))
				{
					switch(selection)
					{
					case 0:
						return GS_GAME;
					default:
						return -1;
					}
				}
			}

			for (i = 0; i < menuItemCount; ++i)
			{
				if (selection == i)
				{
					DrawTextColored(menuItems[i], 110, 120 + i * 16, 128, 128, 128);
				}
				else 
				{
					DrawTextColored(menuItems[i], 110, 120 + i * 16, 64, 64, 64);
				}
			}
		}

		DrawText("Copyright (C) 2022, Kyoril", 64, 188);
		DrawText("All rights reserved.", 84, 208);

		DrawSprite(&titleSprite);

		EndFrame();
	}

	return 1;
}

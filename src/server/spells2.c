/* File: spells2.c */

/* Purpose: Spell code (part 2) */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#define SERVER

#include "angband.h"


/* Wipe everything */
void wipe_spell(int Depth, int cy, int cx, int r)
{
	int		yy, xx, dy, dx;

	cave_type	*c_ptr;



	/* Don't hurt town or surrounding areas */
	if (Depth <= 0 ? wild_info[Depth].radius <= 2 : 0)
		return;

	/* Paranoia -- Enforce maximum range */
	if (r > 12) r = 12;

	/* Check around the epicenter */
	for (dy = -r; dy <= r; dy++)
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip illegal grids */
			if (!in_bounds(Depth, yy, xx)) continue;

			/* Skip distant grids */
			if (distance(cy, cx, yy, xx) > r) continue;

			/* Access the grid */
			c_ptr = &cave[Depth][yy][xx];

			/* Lose room and vault */
			if (Depth > 0)
            		{
                		c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);
            		}
			
			/* Delete monsters */
			delete_monster(Depth, yy, xx);

			/* Destroy "valid" grids */
			if ((cave_valid_bold(Depth, yy, xx)) && !(c_ptr->info & CAVE_ICKY))
			{
				/* Turn into basic floor */
				c_ptr->feat = FEAT_FLOOR;
			
				/* Delete objects */
				delete_object(Depth, yy, xx);
			}

			everyone_lite_spot(Depth, yy, xx);
		}
}


/*
 * Increase players hit points, notice effects, and tell the player about it.
 */
bool hp_player(int Ind, int num)
{
	player_type *p_ptr = Players[Ind];

	// The "number" that the character is displayed as before healing
	int old_num = (p_ptr->chp * 95) / (p_ptr->mhp*10); 
	int new_num; 


	if (p_ptr->chp < p_ptr->mhp)
	{
		p_ptr->chp += num;

		if (p_ptr->chp > p_ptr->mhp)
		{
			p_ptr->chp = p_ptr->mhp;
			p_ptr->chp_frac = 0;
		}

		/* Update health bars */
		update_health(0 - Ind);

		/* Redraw */
		p_ptr->redraw |= (PR_HP);

		/* Figure out of if the player's "number" has changed */
		new_num = (p_ptr->chp * 95) / (p_ptr->mhp*10); 
		if (new_num >= 7) new_num = 10;

		/* If so then refresh everyone's view of this player */
		if (new_num != old_num)
			everyone_lite_spot(p_ptr->dun_depth, p_ptr->py, p_ptr->px);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

		num = num / 5;
		if (num < 3)
		{
			if (num == 0)
			{
				msg_print(Ind, "You feel a little better.");
			}
			else
			{
				msg_print(Ind, "You feel better.");
			}
		}
		else
		{
			if (num < 7)
			{
				msg_print(Ind, "You feel much better.");
			}
			else
			{
				msg_print(Ind, "You feel very good.");
			}
		}

		return (TRUE);
	}

	return (FALSE);
}

/*
 * Increase players hit points, notice effects, and don't tell the player it.
 */
bool hp_player_quiet(int Ind, int num)
{
	player_type *p_ptr = Players[Ind];

	// The "number" that the character is displayed as before healing
	int old_num = (p_ptr->chp * 95) / (p_ptr->mhp*10); 
	int new_num; 


	if (p_ptr->chp < p_ptr->mhp)
	{
		p_ptr->chp += num;

		if (p_ptr->chp > p_ptr->mhp)
		{
			p_ptr->chp = p_ptr->mhp;
			p_ptr->chp_frac = 0;
		}

		/* Update health bars */
		update_health(0 - Ind);

		/* Redraw */
		p_ptr->redraw |= (PR_HP);

		/* Figure out of if the player's "number" has changed */
		new_num = (p_ptr->chp * 95) / (p_ptr->mhp*10); 
		if (new_num >= 7) new_num = 10;

		/* If so then refresh everyone's view of this player */
		if (new_num != old_num)
			everyone_lite_spot(p_ptr->dun_depth, p_ptr->py, p_ptr->px);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

		return (TRUE);
	}

	return (FALSE);
}




/*
 * Leave a "glyph of warding" which prevents monster movement
 */
bool warding_glyph(int Ind)
{
	player_type *p_ptr = Players[Ind];

	cave_type *c_ptr;

	/* Can't create in town */
	if (p_ptr->dun_depth == 0)
	{
		msg_print(Ind, "The very soil of the Town prevents you.");
		return FALSE;
	}

	/* Require clean space */
	if (!cave_clean_bold(p_ptr->dun_depth, p_ptr->py, p_ptr->px))
	{
		msg_print(Ind, "The object resists the spell.");
		return FALSE;
	}

	/* Access the player grid */
	c_ptr = &cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px];

	/* Don't allow glyphs inside houses */
	if (p_ptr->dun_depth < 0 && c_ptr->info & CAVE_ICKY)
	{
		msg_print(Ind, "The floor of the house resists your spell.");
		return FALSE;
	}

	/* Create a glyph of warding */
	c_ptr->feat = FEAT_GLYPH;

  return TRUE;
}




/*
 * Array of stat "descriptions"
 */
static cptr desc_stat_pos[] =
{
	"strong",
	"smart",
	"wise",
	"dextrous",
	"healthy",
	"cute"
};


/*
 * Array of stat "descriptions"
 */
static cptr desc_stat_neg[] =
{
	"weak",
	"stupid",
	"naive",
	"clumsy",
	"sickly",
	"ugly"
};


/*
 * Lose a "point"
 */
bool do_dec_stat(int Ind, int stat)
{
	player_type *p_ptr = Players[Ind];

	bool sust = FALSE;

	/* Access the "sustain" */
	switch (stat)
	{
		case A_STR: if (p_ptr->sustain_str) sust = TRUE; break;
		case A_INT: if (p_ptr->sustain_int) sust = TRUE; break;
		case A_WIS: if (p_ptr->sustain_wis) sust = TRUE; break;
		case A_DEX: if (p_ptr->sustain_dex) sust = TRUE; break;
		case A_CON: if (p_ptr->sustain_con) sust = TRUE; break;
		case A_CHR: if (p_ptr->sustain_chr) sust = TRUE; break;
	}

	/* Sustain */
	if (sust)
	{
		/* Message */
		msg_format(Ind, "You feel %s for a moment, but the feeling passes.",
		           desc_stat_neg[stat]);

		/* Notice effect */
		return (TRUE);
	}

	/* Attempt to reduce the stat */
	if (dec_stat(Ind, stat, 10, FALSE))
	{
		/* Message */
		msg_format(Ind, "You feel very %s.", desc_stat_neg[stat]);

		/* Notice effect */
		return (TRUE);
	}

	/* Nothing obvious */
	return (FALSE);
}


/*
 * Restore lost "points" in a stat
 */
bool do_res_stat(int Ind, int stat)
{
	/* Attempt to increase */
	if (res_stat(Ind, stat))
	{
		/* Message */
		msg_format(Ind, "You feel less %s.", desc_stat_neg[stat]);

		/* Notice */
		return (TRUE);
	}

	/* Nothing obvious */
	return (FALSE);
}


/*
 * Gain a "point" in a stat
 */
bool do_inc_stat(int Ind, int stat)
{
	bool res;

	/* Restore strength */
	res = res_stat(Ind, stat);

	/* Attempt to increase */
	if (inc_stat(Ind, stat))
	{
		/* Message */
		msg_format(Ind, "Wow!  You feel very %s!", desc_stat_pos[stat]);

		/* Notice */
		return (TRUE);
	}

	/* Restoration worked */
	if (res)
	{
		/* Message */
		msg_format(Ind, "You feel less %s.", desc_stat_neg[stat]);

		/* Notice */
		return (TRUE);
	}

	/* Nothing obvious */
	return (FALSE);
}



/*
 * Identify everything being carried.
 * Done by a potion of "self knowledge".
 */
void identify_pack(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int                 i;
	object_type        *o_ptr;

	/* Simply identify and know every item */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		o_ptr = &p_ptr->inventory[i];
		if (o_ptr->k_idx)
		{
			object_aware(Ind, o_ptr);
			object_known(o_ptr);
		}
	}
}






/*
 * Used by the "enchant" function (chance of failure)
 */
static int enchant_table[16] =
{
	0, 10,  50, 100, 200,
	300, 400, 500, 700, 950,
	990, 992, 995, 997, 999,
	1000
};


/*
 * Removes curses from items in inventory
 *
 * Note that Items which are "Perma-Cursed" (The One Ring,
 * The Crown of Morgoth) can NEVER be uncursed.
 *
 * Note that if "all" is FALSE, then Items which are
 * "Heavy-Cursed" (Mormegil, Calris, and Weapons of Morgul)
 * will not be uncursed.
 */
static int remove_curse_aux(int Ind, int all)
{
	player_type *p_ptr = Players[Ind];

	int		i, cnt = 0;

	/* Attempt to uncurse items being worn */
	for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
	        u32b f1, f2, f3;

		object_type *o_ptr = &p_ptr->inventory[i];

		/* Uncursed already */
		if (!cursed_p(o_ptr)) continue;

		/* Extract the flags */
    		object_flags(o_ptr, &f1, &f2, &f3);

		/* Heavily Cursed Items need a special spell */
		if (!all && (f3 & TR3_HEAVY_CURSE)) continue;

		/* Perma-Cursed Items can NEVER be uncursed */
		if (f3 & TR3_PERMA_CURSE) continue;

		/* Uncurse it */
		o_ptr->ident &= ~ID_CURSED;

		/* Hack -- Assume felt */
		o_ptr->ident |= ID_SENSE;

		/* Take note */
		o_ptr->note = quark_add("uncursed");

		/* Recalculate the bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Window stuff */
		p_ptr->window |= (PW_INVEN | PW_EQUIP);

		/* Count the uncursings */
		cnt++;
	}

	/* Return "something uncursed" */
	return (cnt);
}


/*
 * Remove most curses
 */
bool remove_curse(int Ind)
{
	return (remove_curse_aux(Ind, FALSE));
}

/*
 * Remove all curses
 */
bool remove_all_curse(int Ind)
{
	return (remove_curse_aux(Ind, TRUE));
}



/*
 * Restores any drained experience
 */
bool restore_level(int Ind)
{
	player_type *p_ptr = Players[Ind];

	/* Restore experience */
	if (p_ptr->exp < p_ptr->max_exp)
	{
		/* Message */
		msg_print(Ind, "You feel your life energies returning.");

		/* Restore the experience */
		p_ptr->exp = p_ptr->max_exp;

		/* Check the experience */
		check_experience(Ind);

		/* Did something */
		return (TRUE);
	}

	/* No effect */
	return (FALSE);
}


/*
 * self-knowledge... idea from nethack.  Useful for determining powers and
 * resistences of items.  It saves the screen, clears it, then starts listing
 * attributes, a screenful at a time.  (There are a LOT of attributes to
 * list.  It will probably take 2 or 3 screens for a powerful character whose
 * using several artifacts...) -CFT
 *
 * It is now a lot more efficient. -BEN-
 *
 * See also "identify_fully()".
 *
 * XXX XXX XXX Use the "show_file()" method, perhaps.
 */
void self_knowledge(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int		i = 0, k;

	u32b	f1 = 0L, f2 = 0L, f3 = 0L;

	object_type	*o_ptr;

	cptr	*info = p_ptr->info;

	/* Clear the info area first. */
	memset(p_ptr->info,0,sizeof(p_ptr->info));

	/* Let the player scroll through the info */
	p_ptr->special_file_type = TRUE;

	/* Acquire item flags from equipment */
	for (k = INVEN_WIELD; k < INVEN_TOTAL; k++)
	{
	        u32b t1, t2, t3;

		o_ptr = &p_ptr->inventory[k];

		/* Skip empty items */
		if (!o_ptr->k_idx) continue;

		/* Extract the flags */
	        object_flags(o_ptr, &t1, &t2, &t3);

		/* Extract flags */
		f1 |= t1;
		f2 |= t2;
		f3 |= t3;
	}


	if (p_ptr->blind)
	{
		info[i++] = "You cannot see.";
	}
	if (p_ptr->confused)
	{
		info[i++] = "You are confused.";
	}
	if (p_ptr->afraid)
	{
		info[i++] = "You are terrified.";
	}
	if (p_ptr->cut)
	{
		info[i++] = "You are bleeding.";
	}
	if (p_ptr->stun)
	{
		info[i++] = "You are stunned.";
	}
	if (p_ptr->poisoned)
	{
		info[i++] = "You are poisoned.";
	}
	if (p_ptr->image)
	{
		info[i++] = "You are hallucinating.";
	}

	if (p_ptr->aggravate)
	{
		info[i++] = "You aggravate monsters.";
	}
	if (p_ptr->teleport)
	{
        info[i++] = "You are teleporting.";
	}

	if (p_ptr->blessed)
	{
        info[i++] = "You feel righteous.";
	}
	if (p_ptr->hero)
	{
        info[i++] = "You feel like a hero.";
	}
	if (p_ptr->shero)
	{
		info[i++] = "You are in a battle rage.";
	}
	if (p_ptr->protevil)
	{
		info[i++] = "You are protected from evil.";
	}
	if (p_ptr->shield)
	{
		info[i++] = "You are protected by a mystic shield.";
	}
	if (p_ptr->invuln)
	{
		info[i++] = "You are temporarily invulnerable.";
	}
	if (p_ptr->confusing)
	{
		info[i++] = "Your hands are glowing dull red.";
	}
	if (p_ptr->searching)
	{
		info[i++] = "You are looking around very carefully.";
	}
	if (p_ptr->new_spells)
	{
		info[i++] = "You can learn some more spells.";
	}
	if (p_ptr->word_recall)
	{
		info[i++] = "You will soon be recalled.";
	}
	if (p_ptr->see_infra)
	{
		info[i++] = "Your eyes are sensitive to infrared light.";
	}
	if (p_ptr->see_inv)
	{
		info[i++] = "You can see invisible creatures.";
	}
	if (p_ptr->feather_fall)
	{
		info[i++] = "You land gently.";
	}
	if (p_ptr->free_act)
	{
		info[i++] = "You have free action.";
	}
	if (p_ptr->regenerate)
	{
		info[i++] = "You regenerate quickly.";
	}
	if (p_ptr->slow_digest)
	{
		info[i++] = "Your appetite is small.";
	}
    if (p_ptr->telepathy == TR3_TELEPATHY)
	{
		info[i++] = "You have ESP.";
	}
	if (p_ptr->hold_life)
	{
		info[i++] = "You have a firm hold on your life force.";
	}
	if (p_ptr->lite)
	{
		info[i++] = "You are carrying a permanent light.";
	}
	if (p_ptr->immune_acid)
	{
		info[i++] = "You are completely immune to acid.";
	}
	else if ((p_ptr->resist_acid) && (p_ptr->oppose_acid))
	{
		info[i++] = "You resist acid exceptionally well.";
	}
	else if ((p_ptr->resist_acid) || (p_ptr->oppose_acid))
	{
		info[i++] = "You are resistant to acid.";
	}

	if (p_ptr->immune_elec)
	{
		info[i++] = "You are completely immune to lightning.";
	}
	else if ((p_ptr->resist_elec) && (p_ptr->oppose_elec))
	{
		info[i++] = "You resist lightning exceptionally well.";
	}
	else if ((p_ptr->resist_elec) || (p_ptr->oppose_elec))
	{
		info[i++] = "You are resistant to lightning.";
	}

	if (p_ptr->immune_fire)
	{
		info[i++] = "You are completely immune to fire.";
	}
	else if ((p_ptr->resist_fire) && (p_ptr->oppose_fire))
	{
		info[i++] = "You resist fire exceptionally well.";
	}
	else if ((p_ptr->resist_fire) || (p_ptr->oppose_fire))
	{
		info[i++] = "You are resistant to fire.";
	}

	if (p_ptr->immune_cold)
	{
		info[i++] = "You are completely immune to cold.";
	}
	else if ((p_ptr->resist_cold) && (p_ptr->oppose_cold))
	{
		info[i++] = "You resist cold exceptionally well.";
	}
	else if ((p_ptr->resist_cold) || (p_ptr->oppose_cold))
	{
		info[i++] = "You are resistant to cold.";
	}

	if ((p_ptr->resist_pois) && (p_ptr->oppose_pois))
	{
		info[i++] = "You resist poison exceptionally well.";
	}
	else if ((p_ptr->resist_pois) || (p_ptr->oppose_pois))
	{
		info[i++] = "You are resistant to poison.";
	}

	if (p_ptr->resist_lite)
	{
		info[i++] = "You are resistant to bright light.";
	}
	if (p_ptr->resist_dark)
	{
		info[i++] = "You are resistant to darkness.";
	}
	if (p_ptr->resist_conf)
	{
		info[i++] = "You are resistant to confusion.";
	}
	if (p_ptr->resist_sound)
	{
		info[i++] = "You are resistant to sonic attacks.";
	}
	if (p_ptr->resist_disen)
	{
		info[i++] = "You are resistant to disenchantment.";
	}
	if (p_ptr->resist_chaos)
	{
		info[i++] = "You are resistant to chaos.";
	}
	if (p_ptr->resist_shard)
	{
		info[i++] = "You are resistant to blasts of shards.";
	}
	if (p_ptr->resist_nexus)
	{
		info[i++] = "You are resistant to nexus attacks.";
	}
	if (p_ptr->resist_neth)
	{
		info[i++] = "You are resistant to nether forces.";
	}
	if (p_ptr->resist_fear)
	{
		info[i++] = "You are completely fearless.";
	}
	if (p_ptr->resist_blind)
	{
		info[i++] = "Your eyes are resistant to blindness.";
	}

	if (p_ptr->sustain_str)
	{
		info[i++] = "Your strength is sustained.";
	}
	if (p_ptr->sustain_int)
	{
		info[i++] = "Your intelligence is sustained.";
	}
	if (p_ptr->sustain_wis)
	{
		info[i++] = "Your wisdom is sustained.";
	}
	if (p_ptr->sustain_con)
	{
		info[i++] = "Your constitution is sustained.";
	}
	if (p_ptr->sustain_dex)
	{
		info[i++] = "Your dexterity is sustained.";
	}
	if (p_ptr->sustain_chr)
	{
		info[i++] = "Your charisma is sustained.";
	}

	if (f1 & TR1_STR)
	{
		info[i++] = "Your strength is affected by your equipment.";
	}
	if (f1 & TR1_INT)
	{
		info[i++] = "Your intelligence is affected by your equipment.";
	}
	if (f1 & TR1_WIS)
	{
		info[i++] = "Your wisdom is affected by your equipment.";
	}
	if (f1 & TR1_DEX)
	{
		info[i++] = "Your dexterity is affected by your equipment.";
	}
	if (f1 & TR1_CON)
	{
		info[i++] = "Your constitution is affected by your equipment.";
	}
	if (f1 & TR1_CHR)
	{
		info[i++] = "Your charisma is affected by your equipment.";
	}

	if (f1 & TR1_STEALTH)
	{
		info[i++] = "Your stealth is affected by your equipment.";
	}
	if (f1 & TR1_SEARCH)
	{
		info[i++] = "Your searching ability is affected by your equipment.";
	}
	if (f1 & TR1_INFRA)
	{
		info[i++] = "Your infravision is affected by your equipment.";
	}
	if (f1 & TR1_TUNNEL)
	{
		info[i++] = "Your digging ability is affected by your equipment.";
	}
	if (f1 & TR1_SPEED)
	{
		info[i++] = "Your speed is affected by your equipment.";
	}
	if (f1 & TR1_BLOWS)
	{
		info[i++] = "Your attack speed is affected by your equipment.";
	}


	/* Access the current weapon */
	o_ptr = &p_ptr->inventory[INVEN_WIELD];

	/* Analyze the weapon */
	if (o_ptr->k_idx)
	{
		/* Indicate Blessing */
		if (f3 & TR3_BLESSED)
		{
			info[i++] = "Your weapon has been blessed by the gods.";
		}

		/* Hack */
		if (f3 & TR3_IMPACT)
		{
			info[i++] = "The impact of your weapon can cause earthquakes.";
		}

		/* Special "Attack Bonuses" */
		if (f1 & TR1_BRAND_ACID)
		{
			info[i++] = "Your weapon melts your foes.";
		}
		if (f1 & TR1_BRAND_ELEC)
		{
			info[i++] = "Your weapon shocks your foes.";
		}
		if (f1 & TR1_BRAND_FIRE)
		{
			info[i++] = "Your weapon burns your foes.";
		}
		if (f1 & TR1_BRAND_COLD)
		{
			info[i++] = "Your weapon freezes your foes.";
		}
        if (f1 & TR1_BRAND_POIS)
        {
            info[i++] = "Your weapon poisons your foes.";
        }

		/* Special "slay" flags */
		if (f1 & TR1_SLAY_ANIMAL)
		{
			info[i++] = "Your weapon strikes at animals with extra force.";
		}
		if (f1 & TR1_SLAY_EVIL)
		{
			info[i++] = "Your weapon strikes at evil with extra force.";
		}
		if (f1 & TR1_SLAY_UNDEAD)
		{
			info[i++] = "Your weapon strikes at undead with holy wrath.";
		}
		if (f1 & TR1_SLAY_DEMON)
		{
			info[i++] = "Your weapon strikes at demons with holy wrath.";
		}
		if (f1 & TR1_SLAY_ORC)
		{
			info[i++] = "Your weapon is especially deadly against orcs.";
		}
		if (f1 & TR1_SLAY_TROLL)
		{
			info[i++] = "Your weapon is especially deadly against trolls.";
		}
		if (f1 & TR1_SLAY_GIANT)
		{
			info[i++] = "Your weapon is especially deadly against giants.";
		}
		if (f1 & TR1_SLAY_DRAGON)
		{
			info[i++] = "Your weapon is especially deadly against dragons.";
		}

		/* Special "kill" flags */
		if (f1 & TR1_KILL_DRAGON)
		{
			info[i++] = "Your weapon is a great bane of dragons.";
		}
        if (f1 & TR1_KILL_DEMON)
        {
            info[i++] = "Your weapon is a great bane of demons.";
        }
        if (f1 & TR1_KILL_UNDEAD)
        {
            info[i++] = "Your weapon is a great bane of undead.";
        }
	}


	/* Let the client know to expect some info */
	Send_special_other(Ind, "Self-Knowledge");
}






/*
 * Forget everything
 */
bool lose_all_info(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int                 i;

	/* Forget info about objects */
	for (i = 0; i < INVEN_TOTAL; i++)
	{
		object_type *o_ptr = &p_ptr->inventory[i];

		/* Skip non-items */
		if (!o_ptr->k_idx) continue;

		/* Allow "protection" by the MENTAL flag */
		if (o_ptr->ident & ID_MENTAL) continue;

		/* Remove "default inscriptions" */
		if (o_ptr->note && (o_ptr->ident & ID_SENSE))
		{
			/* Access the inscription */
			cptr q = quark_str(o_ptr->note);

			/* Hack -- Remove auto-inscriptions */
			if ((streq(q, "cursed")) ||
			    (streq(q, "broken")) ||
			    (streq(q, "good")) ||
			    (streq(q, "average")) ||
			    (streq(q, "excellent")) ||
			    (streq(q, "worthless")) ||
			    (streq(q, "special")) ||
			    (streq(q, "terrible")))
			{
				/* Forget the inscription */
				o_ptr->note = 0;
			}
		}

		/* Hack -- Clear the "empty" flag */
		o_ptr->ident &= ~ID_EMPTY;

		/* Hack -- Clear the "known" flag */
		o_ptr->ident &= ~ID_KNOWN;

		/* Hack -- Clear the "felt" flag */
		o_ptr->ident &= ~ID_SENSE;
	}

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Mega-Hack -- Forget the map */
	wiz_dark(Ind);

	/* It worked */
	return (TRUE);
}

/*
 * Set word of recall as appropriate
 */
void set_recall(int Ind, object_type * o_ptr)
{
	int recall_depth = 0;
	player_type * p_ptr = Players[Ind];
	unsigned char * inscription = (unsigned char *) quark_str(o_ptr->note);

	/* Ironmen don't recall unless they've won */
	if (cfg_ironman && !p_ptr->total_winner)
	{
		msg_print(Ind,"Nothing happens.");
		return;
	}
	
	/* Activate recall */
	if (!p_ptr->word_recall)
	{
		/* default to the players maximum depth */
		p_ptr->recall_depth = p_ptr->max_dlv;
	
		/* scan the inscription for @R */
		if(inscription)
		{
			while (*inscription != '\0')
			{
				if (*inscription == '@')
				{
					inscription++;
					if (*inscription == 'R')
					{			
						/* a valid @R has been located */
						inscription++;
						/* convert the inscription into a level index */
						recall_depth = atoi(inscription);
						if(!(recall_depth % 50)) 
						{
							recall_depth/=50;
						}
					}
				}
				inscription++;
			}
		}
	
		/* do some bounds checking / sanity checks */
		if((recall_depth > p_ptr->max_dlv) || (!recall_depth)) recall_depth = p_ptr->max_dlv;
	
		/* if a wilderness level, verify that the player has visited here before */
		if (recall_depth < 0)
		{
			/* if the player has not visited here, set the recall depth to the town */
			if ((strcmp(p_ptr->name,cfg_dungeon_master)))
				if (!(p_ptr->wild_map[-recall_depth/8] & (1 << -recall_depth%8)))
					recall_depth = 1;

		}
	
		p_ptr->recall_depth = recall_depth;
		p_ptr->word_recall = (s16b)rand_int(20) + 15;
		msg_print(Ind, "The air about you becomes charged...");
	}
	else
	{
		p_ptr->word_recall = 0;
		msg_print(Ind, "A tension leaves the air around you...");
	}	

}


/*
 * Detect any treasure on the current panel		-RAK-
 *
 * We do not yet create any "hidden gold" features XXX XXX XXX
 */
bool detect_treasure(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int Depth = p_ptr->dun_depth;

	int		y, x;
	bool	detect = FALSE;

	cave_type	*c_ptr;
	byte		*w_ptr;

	object_type	*o_ptr;


	/* Scan the current panel */
	for (y = p_ptr->panel_row_min; y <= p_ptr->panel_row_max; y++)
	{
		for (x = p_ptr->panel_col_min; x <= p_ptr->panel_col_max; x++)
		{
			c_ptr = &cave[Depth][y][x];
			w_ptr = &p_ptr->cave_flag[y][x];

			o_ptr = &o_list[c_ptr->o_idx];

			/* Magma/Quartz + Known Gold */
			if ((c_ptr->feat == FEAT_MAGMA_K) ||
			    (c_ptr->feat == FEAT_QUARTZ_K))
			{
				/* Notice detected gold */
				if (!(*w_ptr & CAVE_MARK))
				{
					/* Detect */
					detect = TRUE;

					/* Hack -- memorize the feature */
					*w_ptr |= CAVE_MARK;

					/* Redraw */
					lite_spot(Ind, y, x);
				}
			}

			/* Notice embedded gold */
			if ((c_ptr->feat == FEAT_MAGMA_H) ||
			    (c_ptr->feat == FEAT_QUARTZ_H))
			{
				/* Expose the gold */
				c_ptr->feat += 0x02;

				/* Detect */
				detect = TRUE;

				/* Hack -- memorize the item */
				*w_ptr |= CAVE_MARK;

				/* Redraw */
				lite_spot(Ind, y, x);
			}
#if 0
			/* Notice gold */
			if (o_ptr->tval == TV_GOLD)
			{
				/* Notice new items */
				if (!(p_ptr->obj_vis[c_ptr->o_idx]))
				{
					/* Detect */
					detect = TRUE;

					/* Hack -- memorize the item */
					p_ptr->obj_vis[c_ptr->o_idx] = TRUE;

					/* Redraw */
					lite_spot(Ind, y, x);
				}
			}
#endif
		}
	}
	
	/* Describe */
	if (detect)
	{
		msg_print(Ind, "You sense the presence of buried treasure!");
	}

	return (detect);
}

/*
 * Detect magic items.
 *
 * This will light up all spaces with "magic" items, including artifacts,
 * ego-items, potions, scrolls, books, rods, wands, staves, amulets, rings,
 * and "enchanted" items of the "good" variety.
 *
 * It can probably be argued that this function is now too powerful.
 */
bool detect_objects_magic(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int Depth = p_ptr->dun_depth;

	int		i, j, tv;
	bool	detect = FALSE;

	cave_type	*c_ptr;
	object_type	*o_ptr;


	/* Scan the current panel */
	for (i = p_ptr->panel_row_min; i <= p_ptr->panel_row_max; i++)
	{
		for (j = p_ptr->panel_col_min; j <= p_ptr->panel_col_max; j++)
		{
			/* Access the grid and object */
			c_ptr = &cave[Depth][i][j];
			o_ptr = &o_list[c_ptr->o_idx];

			/* Nothing there */
			if (!(c_ptr->o_idx)) continue;

			/* Examine the tval */
			tv = o_ptr->tval;

			/* Artifacts, misc magic items, or enchanted wearables */
			if (artifact_p(o_ptr) || ego_item_p(o_ptr) ||
			    (tv == TV_AMULET) || (tv == TV_RING) ||
			    (tv == TV_STAFF) || (tv == TV_WAND) || (tv == TV_ROD) ||
			    (tv == TV_SCROLL) || (tv == TV_POTION) ||
			    (tv == TV_MAGIC_BOOK) || (tv == TV_PRAYER_BOOK) ||
			    ((o_ptr->to_a > 0) || (o_ptr->to_h + o_ptr->to_d > 0)))
			{
				/* Note new items */
				if (!(p_ptr->obj_vis[c_ptr->o_idx]))
				{
					/* Detect */
					detect = TRUE;

					/* Memorize the item */
					p_ptr->obj_vis[c_ptr->o_idx] = TRUE;

					/* Redraw */
					lite_spot(Ind, i, j);
				}
			}
		}
	}

	/* Return result */
	return (detect);
}





/*
 * Locates and displays all invisible creatures on current panel -RAK-
 */
bool detect_invisible(int Ind, bool pause)
{
	player_type *p_ptr = Players[Ind];

	int		i;
	bool	flag = FALSE;


	/* Detect all invisible monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		int fy = m_ptr->fy;
		int fx = m_ptr->fx;

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip visible monsters */
		if (p_ptr->mon_vis[i]) continue;

		/* Skip monsters not on this depth */
		if (m_ptr->dun_depth != p_ptr->dun_depth) continue;

		/* Detect all invisible monsters */
		if (panel_contains(fy, fx) && (r_ptr->flags2 & RF2_INVISIBLE))
		{
			/* Take note that they are invisible */
			r_ptr->r_flags2 |= RF2_INVISIBLE;

			/* Mega-Hack -- Show the monster */
			p_ptr->mon_vis[i] = TRUE;
			lite_spot(Ind, fy, fx);
			flag = TRUE;
		}
	}

	/* Detect all invisible players */
	for (i = 1; i <= NumPlayers; i++)
	{
		player_type *q_ptr = Players[i];

		int py = q_ptr->py;
		int px = q_ptr->px;

		/* Skip visible players */
		if (p_ptr->dun_depth != q_ptr->dun_depth) continue;

		/* Skip the dungeon master */
		if (!strcmp(q_ptr->name, cfg_dungeon_master)) continue;

		/* Detect all invisible players but not the dungeon master */
		if (panel_contains(py, px) && q_ptr->ghost) 
		{
			/* Mega-Hack -- Show the player */
			p_ptr->play_vis[i] = TRUE;
			lite_spot(Ind, py, px);
			flag = TRUE;
		}
	}

	/* Describe result, and clean up */
	if (flag && pause)
	{
		/* Describe, and wait for acknowledgement */
		msg_print(Ind, "You sense the presence of invisible creatures!");
		msg_print(Ind, NULL);

		/* Wait */
		Send_pause(Ind);

		/* Mega-Hack -- Fix the monsters and players */
		update_monsters(FALSE);
		update_players();
	}

	/* Result */
	return (flag);
}



/*
 * Display evil creatures on current panel		-RAK-
 */
bool detect_evil(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int		i;
	bool	flag = FALSE;


	/* Display all the evil monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		int fy = m_ptr->fy;
		int fx = m_ptr->fx;

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip monsters not on this depth */
		if (m_ptr->dun_depth != p_ptr->dun_depth) continue;

		/* Detect evil monsters */
		if (panel_contains(fy, fx) && (r_ptr->flags3 & RF3_EVIL))
		{
			/* Mega-Hack -- Show the monster */
			p_ptr->mon_vis[i] = TRUE;
			lite_spot(Ind, fy, fx);
			flag = TRUE;
		}
	}

	/* Note effects and clean up */
	if (flag)
	{
		/* Describe, and wait for acknowledgement */
		msg_print(Ind, "You sense the presence of evil creatures!");
		msg_print(Ind, NULL);

		/* Wait */
		Send_pause(Ind);

		/* Mega-Hack -- Fix the monsters */
		update_monsters(FALSE);
	}

	/* Result */
	return (flag);
}



/*
 * Display all non-invisible monsters/players on the current panel
 */
bool detect_creatures(int Ind, bool pause)
{
	player_type *p_ptr = Players[Ind];

	int		i;
	bool	flag = FALSE;


	/* Detect non-invisible monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		int fy = m_ptr->fy;
		int fx = m_ptr->fx;

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip visible monsters */
		if (p_ptr->mon_vis[i]) continue;

		/* Skip monsters not on this depth */
		if (m_ptr->dun_depth != p_ptr->dun_depth) continue;

		/* Detect all non-invisible monsters */
		if (panel_contains(fy, fx) && (!(r_ptr->flags2 & RF2_INVISIBLE)))
		{
			/* Mega-Hack -- Show the monster */
			p_ptr->mon_vis[i] = TRUE;
			lite_spot(Ind, fy, fx);
			flag = TRUE;
		}
	}

	/* Detect non-invisible players */
	for (i = 1; i <= NumPlayers; i++)
	{
		player_type *q_ptr = Players[i];

		int py = q_ptr->py;
		int px = q_ptr->px;

		/* Skip visible players */
		if (p_ptr->play_vis[i]) continue;

		/* Skip players not on this depth */
		if (p_ptr->dun_depth != q_ptr->dun_depth) continue;

		/* Skip ourself */
		if (i == Ind) continue;

		/* Detect all non-invisible players */
		if (panel_contains(py, px) && !q_ptr->ghost)
		{
			/* Mega-Hack -- Show the player */
			p_ptr->play_vis[i] = TRUE;
			lite_spot(Ind, py, px);
			flag = TRUE;
		}
	}

	/* Describe and clean up */
	if (flag && pause)
	{
		/* Describe, and wait for acknowledgement */
		msg_print(Ind, "You sense the presence of creatures!");
		msg_print(Ind, NULL);

		/* Wait */
		Send_pause(Ind);

		/* Mega-Hack -- Fix the monsters and players */
		update_monsters(FALSE);
		update_players();
	}

	/* Result */
	return (flag);
}


/*
 * Detect everything
 */
bool detection(int Ind)
{
	bool	detect = FALSE;
	bool	detected_invis, detected_creatures = FALSE;

	/* Detect the easy things */
	if (detect_treasure(Ind)) detect = TRUE;
	if (detect_objects_normal(Ind)) detect = TRUE;
	if (detect_trap(Ind)) detect = TRUE;
	if (detect_sdoor(Ind)) detect = TRUE;
	detected_creatures = detect_creatures(Ind, FALSE);
	detected_invis = detect_invisible(Ind, FALSE);
		
	/* Describe result, and clean up */
	if (detected_creatures || detected_invis)
	{
		detect = TRUE;
		/* Describe, and wait for acknowledgement */
		msg_print(Ind, "You sense the presence of creatures!");
		msg_print(Ind, NULL);

		/* Wait */
		Send_pause(Ind);

		/* Mega-Hack -- Fix the monsters and players */
		update_monsters(FALSE);
		update_players();
	}

	/* Result */
	return (detect);
}


/*
 * Detect all objects on the current panel		-RAK-
 */
bool detect_objects_normal(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int Depth = p_ptr->dun_depth;

	int		i, j;
	bool	detect = FALSE;

	cave_type	*c_ptr;

	object_type	*o_ptr;


	/* Scan the current panel */
	for (i = p_ptr->panel_row_min; i <= p_ptr->panel_row_max; i++)
	{
		for (j = p_ptr->panel_col_min; j <= p_ptr->panel_col_max; j++)
		{
			c_ptr = &cave[Depth][i][j];

			o_ptr = &o_list[c_ptr->o_idx];

			/* Nothing here */
			if (!(c_ptr->o_idx)) continue;

			/* Do not detect "gold" */
			if (o_ptr->tval == TV_GOLD) continue;

			/* Note new objects */
			if (!(p_ptr->obj_vis[c_ptr->o_idx]))
			{
				/* Detect */
				detect = TRUE;

				/* Hack -- memorize it */
				p_ptr->obj_vis[c_ptr->o_idx] = TRUE;

				/* Redraw */
				lite_spot(Ind, i, j);
			}
		}
	}

	return (detect);
}


/*
 * Locates and displays traps on current panel
 */
bool detect_trap(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int Depth = p_ptr->dun_depth;

	int		i, j;

	bool	detect = FALSE;

	cave_type  *c_ptr;
	byte *w_ptr;


	/* Scan the current panel */
	for (i = p_ptr->panel_row_min; i <= p_ptr->panel_row_max; i++)
	{
		for (j = p_ptr->panel_col_min; j <= p_ptr->panel_col_max; j++)
		{
			/* Access the grid */
			c_ptr = &cave[Depth][i][j];
			w_ptr = &p_ptr->cave_flag[i][j];

			/* Detect invisible traps */
			if (c_ptr->feat == FEAT_INVIS)
			{
				/* Pick a trap */
				pick_trap(Depth, i, j);

				/* Hack -- memorize it */
				*w_ptr |= CAVE_MARK;

				/* Redraw */
				lite_spot(Ind, i, j);

				/* Obvious */
				detect = TRUE;
			}

			/* Already seen traps */
			else if (c_ptr->feat >= FEAT_TRAP_HEAD && c_ptr->feat <= FEAT_TRAP_TAIL)
			{
				/* Memorize it */
				*w_ptr |= CAVE_MARK;

				/* Redraw */
				lite_spot(Ind, i, j);

				/* Obvious */
				detect = TRUE;
			}
		}
	}

	return (detect);
}



/*
 * Locates and displays all stairs and secret doors on current panel -RAK-
 */
bool detect_sdoor(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int Depth = p_ptr->dun_depth;

	int		i, j;
	bool	detect = FALSE;

	cave_type *c_ptr;
	byte *w_ptr;


	/* Scan the panel */
	for (i = p_ptr->panel_row_min; i <= p_ptr->panel_row_max; i++)
	{
		for (j = p_ptr->panel_col_min; j <= p_ptr->panel_col_max; j++)
		{
			/* Access the grid and object */
			c_ptr = &cave[Depth][i][j];
			w_ptr = &p_ptr->cave_flag[i][j];

			/* Hack -- detect secret doors */
			if (c_ptr->feat == FEAT_SECRET)
			{
				/* Find the door XXX XXX XXX */
				c_ptr->feat = FEAT_DOOR_HEAD + 0x00;

				/* Memorize the door */
				*w_ptr |= CAVE_MARK;

				/* Redraw */
				lite_spot(Ind, i, j);

				/* Obvious */
				detect = TRUE;
			}
			else if (((c_ptr->feat >= FEAT_DOOR_HEAD) &&
			     (c_ptr->feat <= FEAT_DOOR_TAIL)) ||
			    ((c_ptr->feat == FEAT_OPEN) ||
			     (c_ptr->feat == FEAT_BROKEN)))
			{ /* Deal with non-hidden doors as well */
				/* Memorize the door */
				*w_ptr |= CAVE_MARK;

				/* Redraw */
				lite_spot(Ind, i, j);

				/* Obvious */
				detect = TRUE;
			}

			/* Ignore known grids */
			if (*w_ptr & CAVE_MARK) continue;

			/* Hack -- detect stairs */
			if ((c_ptr->feat == FEAT_LESS) ||
			    (c_ptr->feat == FEAT_MORE))
			{
				/* Memorize the stairs */
				*w_ptr |= CAVE_MARK;

				/* Redraw */
				lite_spot(Ind, i, j);

				/* Obvious */
				detect = TRUE;
			}
		}
	}

	return (detect);
}


/*
 * Create stairs at the player location
 */
void stair_creation(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int Depth = p_ptr->dun_depth;

	/* Access the grid */
	cave_type *c_ptr;

	if(Depth <= 0 ) { return;};

	/* Access the player grid */
	c_ptr = &cave[Depth][p_ptr->py][p_ptr->px];

	/* forbid perma-grids
	 * forbid grids containing artifacts
	 * forbid house doors
	 */
	if (!cave_valid_bold(Depth, p_ptr->py, p_ptr->px))
	{
		msg_print(Ind, "The object resists the spell.");
		return;
	}

	/* Hack -- Delete old contents */
	delete_object(Depth, p_ptr->py, p_ptr->px);

	/* Create a staircase */
	if (!Depth) /* Should never happen, would be in town */
	{
		c_ptr->feat = FEAT_MORE;
	}
	else if (is_quest(Depth) || (Depth >= MAX_DEPTH-1))
	{
		c_ptr->feat = FEAT_LESS;
	}
	else if (rand_int(100) < 50)
	{
		c_ptr->feat = FEAT_MORE;
	}
	else
	{
		c_ptr->feat = FEAT_LESS;
	}

	/* Notice */
	note_spot(Ind, p_ptr->py, p_ptr->px);

	/* Redraw */
	everyone_lite_spot(Depth, p_ptr->py, p_ptr->px);
}




/*
 * Hook to specify "weapon"
 */
static bool item_tester_hook_weapon(object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_DIGGING:
		case TV_BOW:
		case TV_BOLT:
		case TV_ARROW:
		case TV_SHOT:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Hook to specify "armour"
 */
static bool item_tester_hook_armour(object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_DRAG_ARMOR:
		case TV_HARD_ARMOR:
		case TV_SOFT_ARMOR:
		case TV_SHIELD:
		case TV_CLOAK:
		case TV_CROWN:
		case TV_HELM:
		case TV_BOOTS:
		case TV_GLOVES:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}



/*
 * Enchants a plus onto an item.                        -RAK-
 *
 * Revamped!  Now takes item pointer, number of times to try enchanting,
 * and a flag of what to try enchanting.  Artifacts resist enchantment
 * some of the time, and successful enchantment to at least +0 might
 * break a curse on the item.  -CFT
 *
 * Note that an item can technically be enchanted all the way to +15 if
 * you wait a very, very, long time.  Going from +9 to +10 only works
 * about 5% of the time, and from +10 to +11 only about 1% of the time.
 *
 * Note that this function can now be used on "piles" of items, and
 * the larger the pile, the lower the chance of success.
 */
bool enchant(int Ind, object_type *o_ptr, int n, int eflag)
{
	player_type *p_ptr = Players[Ind];

	int i, chance, prob;

	bool res = FALSE;

	bool a = artifact_p(o_ptr);

    u32b f1, f2, f3;

    /* Magic ammo are always +0 +0 */
    if (((o_ptr->tval == TV_SHOT) || (o_ptr->tval == TV_ARROW) ||
	(o_ptr->tval == TV_BOLT)) && (o_ptr->sval == SV_AMMO_MAGIC))
	return FALSE;

    /* Artifact ammo cannot be enchanted */
    if (((o_ptr->tval == TV_SHOT) || (o_ptr->tval == TV_ARROW) ||
	(o_ptr->tval == TV_BOLT)) && a)
	return FALSE;

	/* Extract the flags */
        object_flags(o_ptr, &f1, &f2, &f3);


	/* Large piles resist enchantment */
	prob = o_ptr->number * 100;

	/* Missiles are easy to enchant */
	if ((o_ptr->tval == TV_BOLT) ||
	    (o_ptr->tval == TV_ARROW) ||
	    (o_ptr->tval == TV_SHOT))
	{
		prob = prob / 20;
	}

	/* Try "n" times */
	for (i=0; i<n; i++)
	{
		/* Hack -- Roll for pile resistance */
		if (rand_int(prob) >= 100) continue;

		/* Enchant to hit */
		if (eflag & ENCH_TOHIT)
		{
			if (o_ptr->to_h < 0) chance = 0;
			else if (o_ptr->to_h > 15) chance = 1000;
			else chance = enchant_table[o_ptr->to_h];

			if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
			{
				o_ptr->to_h++;
				res = TRUE;

				/* only when you get it above -1 -CFT */
				if (cursed_p(o_ptr) &&
				    (!(f3 & TR3_PERMA_CURSE)) &&
				    (o_ptr->to_h >= 0) && (rand_int(100) < 25))
				{
					msg_print(Ind, "The curse is broken!");
					o_ptr->ident &= ~ID_CURSED;
					o_ptr->ident |= ID_SENSE;
					o_ptr->note = quark_add("uncursed");
				}
			}
		}

		/* Enchant to damage */
		if (eflag & ENCH_TODAM)
		{
			if (o_ptr->to_d < 0) chance = 0;
			else if (o_ptr->to_d > 15) chance = 1000;
			else chance = enchant_table[o_ptr->to_d];

			if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
			{
				o_ptr->to_d++;
				res = TRUE;

				/* only when you get it above -1 -CFT */
				if (cursed_p(o_ptr) &&
				    (!(f3 & TR3_PERMA_CURSE)) &&
				    (o_ptr->to_d >= 0) && (rand_int(100) < 25))
				{
					msg_print(Ind, "The curse is broken!");
					o_ptr->ident &= ~ID_CURSED;
					o_ptr->ident |= ID_SENSE;
					o_ptr->note = quark_add("uncursed");
				}
			}
		}

		/* Enchant to armor class */
		if (eflag & ENCH_TOAC)
		{
			if (o_ptr->to_a < 0) chance = 0;
			else if (o_ptr->to_a > 15) chance = 1000;
			else chance = enchant_table[o_ptr->to_a];

			if ((randint(1000) > chance) && (!a || (rand_int(100) < 50)))
			{
				o_ptr->to_a++;
				res = TRUE;

				/* only when you get it above -1 -CFT */
				if (cursed_p(o_ptr) &&
				    (!(f3 & TR3_PERMA_CURSE)) &&
				    (o_ptr->to_a >= 0) && (rand_int(100) < 25))
				{
					msg_print(Ind, "The curse is broken!");
					o_ptr->ident &= ~ID_CURSED;
					o_ptr->ident |= ID_SENSE;
					o_ptr->note = quark_add("uncursed");
				}
			}
		}
	}

	/* Failure */
	if (!res) return (FALSE);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Resend plusses */
	p_ptr->redraw |= (PR_PLUSSES);

	/* Success */
	return (TRUE);
}

void spell_clear(int Ind)
{
  player_type *p_ptr = Players[Ind];
  p_ptr->current_enchant_h = 0;
  p_ptr->current_enchant_d = 0;
  p_ptr->current_enchant_a = 0;
  p_ptr->current_identify = 0;
  p_ptr->current_star_identify = 0;
  p_ptr->current_recharge = 0;
  p_ptr->current_artifact = 0;
  
  /* Hack: this should be somewhere else: */
  p_ptr->current_staff = -1;
  p_ptr->current_scroll = -1;
}

bool create_artifact(int Ind)
{
  player_type *p_ptr = Players[Ind];

  spell_clear(Ind);
  p_ptr->current_artifact = TRUE;

  get_item(Ind);
  
  return TRUE;
}


bool create_artifact_aux(int Ind, int item)
{
	player_type *p_ptr = Players[Ind];

	object_type *o_ptr;
	char o_name[80]; /* Only used by randart() */

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &p_ptr->inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		item = -cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].o_idx;
			if (item == 0) {
			msg_print(Ind, "There's nothing on the floor.");
			return FALSE;
		}
		o_ptr = &o_list[0 - item];
	}
#if !defined(RANDART)
	/* Cheap hack: maximum depth , playerlevel, etc */ 
	apply_magic(127, o_ptr, 75, TRUE, TRUE, TRUE);

	return TRUE;
#else
	if (o_ptr->number > 1) return FALSE;
	if (artifact_p(o_ptr)) return FALSE;
	
	/* Description */
	object_desc(Ind, o_name, o_ptr, FALSE, 0);

	/* Describe */
	msg_format(Ind, "%s %s glow%s brightly!",
		((item >= 0) ? "Your" : "The"), o_name,
		((o_ptr->number > 1) ? "" : "s"));

	o_ptr->name1 = ART_RANDART;

	/* Piece together a 32-bit random seed */
	o_ptr->name3 = rand_int(0xFFFF) << 16;
	o_ptr->name3 += rand_int(0xFFFF);
	/* Check the tval is allowed */
	if (randart_make(o_ptr) == NULL)
	{
		/* If not, wipe seed. No randart today */
		o_ptr->name1 = 0;
		o_ptr->name3 = 0L;

		return FALSE;
	}

	apply_magic(p_ptr->dun_depth, o_ptr, p_ptr->lev, FALSE, FALSE, FALSE);

	/* Remove all inscriptions */
        if (o_ptr->note)
        {
		/* Forget the inscription */
                o_ptr->note = 0;
        }

	/* Clear flags */
	o_ptr->ident &= ~ID_KNOWN;
	o_ptr->ident &= ~ID_SENSE;
	o_ptr->ident &= ~ID_MENTAL;

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	p_ptr->current_artifact = FALSE;
#endif
	return TRUE;
}


bool enchant_spell(int Ind, int num_hit, int num_dam, int num_ac)
{
	player_type *p_ptr = Players[Ind];

	spell_clear(Ind);
	p_ptr->current_enchant_h = num_hit;
	p_ptr->current_enchant_d = num_dam;
	p_ptr->current_enchant_a = num_ac;

	get_item(Ind);
	
	return (TRUE);
}
	
/*
 * Enchant an item (in the inventory or on the floor)
 * Note that "num_ac" requires armour, else weapon
 * Returns TRUE if attempted, FALSE if cancelled
 */
bool enchant_spell_aux(int Ind, int item, int num_hit, int num_dam, int num_ac)
{
	player_type *p_ptr = Players[Ind];

	bool		okay = FALSE;

	object_type		*o_ptr;

	char		o_name[80];


	/* Assume enchant weapon */
	item_tester_hook = item_tester_hook_weapon;

	/* Enchant armor if requested */
	if (num_ac) item_tester_hook = item_tester_hook_armour;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &p_ptr->inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		item = -cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].o_idx;
		if (item == 0) {
			msg_print(Ind, "There's nothing on the floor.");
			return FALSE;
		}
		o_ptr = &o_list[0 - item];
	}


	if (!item_tester_hook(o_ptr))
	{
		msg_print(Ind, "Sorry, you cannot enchant that item.");
		get_item(Ind);
		return (FALSE);
	}

	/* Description */
	object_desc(Ind, o_name, o_ptr, FALSE, 0);

	/* Describe */
	msg_format(Ind, "%s %s glow%s brightly!",
	           ((item >= 0) ? "Your" : "The"), o_name,
	           ((o_ptr->number > 1) ? "" : "s"));

	/* Enchant */
	if (enchant(Ind, o_ptr, num_hit, ENCH_TOHIT)) okay = TRUE;
	if (enchant(Ind, o_ptr, num_dam, ENCH_TODAM)) okay = TRUE;
	if (enchant(Ind, o_ptr, num_ac, ENCH_TOAC)) okay = TRUE;

	/* Failure */
	if (!okay)
	{
		/* Flush */
		/*if (flush_failure) flush();*/

		/* Message */
		msg_print(Ind, "The enchantment failed.");
	}

	p_ptr->current_enchant_h = -1;
	p_ptr->current_enchant_d = -1;
	p_ptr->current_enchant_a = -1;

	/* Something happened */
	return (TRUE);
}



bool ident_spell(int Ind)
{
	player_type *p_ptr = Players[Ind];

	spell_clear(Ind);
	p_ptr->current_identify = 1;

	get_item(Ind);
   
	return TRUE;
}

/*
 * Identify an object in the inventory (or on the floor)
 * This routine does *not* automatically combine objects.
 * Returns TRUE if something was identified, else FALSE.
 */
bool ident_spell_aux(int Ind, int item)
{
	player_type *p_ptr = Players[Ind];

	object_type		*o_ptr;

	char		o_name[80];


	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &p_ptr->inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		item = -cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].o_idx;
		if (item == 0) {
			msg_print(Ind, "There's nothing on the floor.");
			return FALSE;
		}
		o_ptr = &o_list[0 - item];
	}


	/* Identify it fully */
	object_aware(Ind, o_ptr);
	object_known(o_ptr);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Description */
	object_desc(Ind, o_name, o_ptr, TRUE, 3);

	/* Describe */
	if (item >= INVEN_WIELD)
	{
		msg_format(Ind, "%^s: %s (%c).",
		           describe_use(Ind, item), o_name, index_to_label(item));
	}
	else if (item >= 0)
	{
		msg_format(Ind, "In your pack: %s (%c).",
		           o_name, index_to_label(item));
	}
	else
	{
		msg_format(Ind, "On the ground: %s.",
		           o_name);
	}

	p_ptr->current_identify = 0;

	/* Something happened */
	return (TRUE);
}


bool identify_fully(int Ind)
{
	player_type *p_ptr = Players[Ind];

	spell_clear(Ind);
	p_ptr->current_star_identify = 1;

	get_item(Ind);

	return TRUE;
}


/*
 * Fully "identify" an object in the inventory	-BEN-
 * This routine returns TRUE if an item was identified.
 */
bool identify_fully_item(int Ind, int item)
{
	player_type *p_ptr = Players[Ind];

	object_type		*o_ptr;

	char		o_name[80];


	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &p_ptr->inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		item = -cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].o_idx;
		if (item == 0) {
			msg_print(Ind, "There's nothing on the floor.");
			return FALSE;
		}
		o_ptr = &o_list[0 - item];
	}


	/* Identify it fully */
	object_aware(Ind, o_ptr);
	object_known(o_ptr);

	/* Mark the item as fully known */
	o_ptr->ident |= (ID_MENTAL);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_PLAYER);

	/* Handle stuff */
	handle_stuff(Ind);

	/* Description */
	object_desc(Ind, o_name, o_ptr, TRUE, 3);

	/* Describe */
	if (item >= INVEN_WIELD)
	{
		msg_format(Ind, "%^s: %s (%c).",
		           describe_use(Ind, item), o_name, index_to_label(item));
	}
	else if (item >= 0)
	{
		msg_format(Ind, "In your pack: %s (%c).",
		           o_name, index_to_label(item));
	}
	else
	{
		msg_format(Ind, "On the ground: %s.",
		           o_name);
	}

	/* Describe it fully */
	identify_fully_aux(Ind, o_ptr);
	Send_special_other(Ind, o_name);

	/* We no longer have a *identify* in progress */
	p_ptr->current_star_identify = 0;

	/* Success */
	return (TRUE);
}




/*
 * Hook for "get_item()".  Determine if something is rechargable.
 */
static bool item_tester_hook_recharge(object_type *o_ptr)
{
	/* Recharge staffs */
	if (o_ptr->tval == TV_STAFF) return (TRUE);

	/* Recharge wands */
	if (o_ptr->tval == TV_WAND) return (TRUE);

	/* Nope */
	return (FALSE);
}


bool recharge(int Ind, int num)
{
	player_type *p_ptr = Players[Ind];

	spell_clear(Ind);
	p_ptr->current_recharge = num;

	get_item(Ind);

	return TRUE;
}


/*
 * Recharge a wand or staff from the pack or on the floor.
 *
 * Mage -- Recharge I --> recharge(5)
 * Mage -- Recharge II --> recharge(40)
 * Mage -- Recharge III --> recharge(100)
 *
 * Priest -- Recharge --> recharge(15)
 *
 * Scroll of recharging --> recharge(60)
 *
 * recharge(20) = 1/6 failure for empty 10th level wand
 * recharge(60) = 1/10 failure for empty 10th level wand
 *
 * It is harder to recharge high level, and highly charged wands.
 *
 * XXX XXX XXX Beware of "sliding index errors".
 *
 * Should probably not "destroy" over-charged items, unless we
 * "replace" them by, say, a broken stick or some such.  The only
 * reason this is okay is because "scrolls of recharging" appear
 * BEFORE all staffs/wands/rods in the inventory.  Note that the
 * new "auto_sort_pack" option would correctly handle replacing
 * the "broken" wand with any other item (i.e. a broken stick).
 *
 * XXX XXX XXX Perhaps we should auto-unstack recharging stacks.
 */
bool recharge_aux(int Ind, int item, int num)
{
	player_type *p_ptr = Players[Ind];

	int                 i, t, lev;

	object_type		*o_ptr;


	/* Only accept legal items */
	item_tester_hook = item_tester_hook_recharge;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &p_ptr->inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		item = -cave[p_ptr->dun_depth][p_ptr->py][p_ptr->px].o_idx;
		if (item == 0) {
			msg_print(Ind, "There's nothing on the floor.");
			return FALSE;
		}
		o_ptr = &o_list[0 - item];
	}

	if (!item_tester_hook(o_ptr))
	{
		msg_print(Ind, "You cannot recharge that item.");
		get_item(Ind);
		return (FALSE);
	}

	/* Extract the object "level" */
	lev = k_info[o_ptr->k_idx].level;

	/* Recharge power */
	i = (num + 100 - lev - (10 * o_ptr->pval)) / 15;

	/* Paranoia -- prevent crashes */
	if (i < 1) i = 1;

	/* Back-fire XXX XXX XXX */
	if (rand_int(i) == 0)
	{
		msg_print(Ind, "There is a bright flash of light.");

		/* Drain the power */
		o_ptr->pval = 0;

		/* *Identified* items keep the knowledge about the charges */
		if (!(o_ptr->ident & ID_MENTAL))
		{
			/* We no longer "know" the item */
			o_ptr->ident &= ~(ID_KNOWN);
		}

		/* We know that the item is empty */
		o_ptr->ident |= ID_EMPTY;
	}

	/* Recharge */
	else
	{
		/* Extract a "power" */
		t = (num / (lev + 2)) + 1;

		/* Recharge based on the power */
		if (t > 0) o_ptr->pval += 2 + randint(t);

		/* Hack -- we no longer "know" the item */
		o_ptr->ident &= ~ID_KNOWN;

		/* Hack -- we no longer think the item is empty */
		o_ptr->ident &= ~ID_EMPTY;
	}

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP);

	/* We no longer have a recharge in progress */
	p_ptr->current_recharge = 0;

	/* Something was done */
	return (TRUE);
}








/*
 * Apply a "project()" directly to all viewable monsters
 */
static bool project_hack(int Ind, int typ, int dam)
{
	player_type *p_ptr = Players[Ind];

	int Depth = p_ptr->dun_depth;

	int		i, x, y;

	int		flg = PROJECT_JUMP | PROJECT_KILL | PROJECT_HIDE;

	bool	obvious = FALSE;


	/* Affect all (nearby) monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Location */
		y = m_ptr->fy;
		x = m_ptr->fx;

		/* Skip monsters not on this depth */
		if (Depth != m_ptr->dun_depth) continue;

		/* Require line of sight */
		if (!player_has_los_bold(Ind, y, x)) continue;

		/* Jump directly to the target monster */
		if (project(0 - Ind, 0, Depth, y, x, dam, typ, flg)) obvious = TRUE;
	}

	/* Result */
	return (obvious);
}


/*
 * Speed monsters
 */
bool speed_monsters(int Ind)
{
	player_type *p_ptr = Players[Ind];

	return (project_hack(Ind, GF_OLD_SPEED, p_ptr->lev));
}

/*
 * Slow monsters
 */
bool slow_monsters(int Ind)
{
	player_type *p_ptr = Players[Ind];

	return (project_hack(Ind, GF_OLD_SLOW, p_ptr->lev));
}

/*
 * Sleep monsters
 */
bool sleep_monsters(int Ind)
{
	player_type *p_ptr = Players[Ind];

	return (project_hack(Ind, GF_OLD_SLEEP, p_ptr->lev));
}


/*
 * Banish evil monsters
 */
bool banish_evil(int Ind, int dist)
{
	return (project_hack(Ind, GF_AWAY_EVIL, dist));
}


/*
 * Turn undead
 */
bool turn_undead(int Ind)
{
	player_type *p_ptr = Players[Ind];

	return (project_hack(Ind, GF_TURN_UNDEAD, p_ptr->lev));
}


/*
 * Dispel undead monsters
 */
bool dispel_undead(int Ind, int dam)
{
	return (project_hack(Ind, GF_DISP_UNDEAD, dam));
}

/*
 * Dispel evil monsters
 */
bool dispel_evil(int Ind, int dam)
{
	return (project_hack(Ind, GF_DISP_EVIL, dam));
}

/*
 * Dispel all monsters
 */
bool dispel_monsters(int Ind, int dam)
{
	return (project_hack(Ind, GF_DISP_ALL, dam));
}




/*
 * Wake up all monsters, and speed up "los" monsters.
 */
void aggravate_monsters(int Ind, int who)
{
	player_type *p_ptr = Players[Ind];

	int i, d;

	bool sleep = FALSE;
	bool speed = FALSE;

	/* Aggravate everyone nearby */
	for (i = 1; i < m_max; i++)
	{
		monster_type	*m_ptr = &m_list[i];
		monster_race	*r_ptr = &r_info[m_ptr->r_idx];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip monsters not on this depth */
		if (p_ptr->dun_depth != m_ptr->dun_depth) continue;

		/* Skip aggravating monster (or player) */
		if (i == who) continue;

		/* Calculate the distance to this monster */
		d = distance(p_ptr->py, p_ptr->px, m_ptr->fy, m_ptr->fx);

		/* Wake up nearby sleeping monsters */
		if (d < MAX_SIGHT * 2)
		{
			/* Wake up */
			if (m_ptr->csleep)
			{
				/* Wake up */
				m_ptr->csleep = 0;
				sleep = TRUE;
			}
		}

		/* Speed up monsters in line of sight */
		if (player_has_los_bold(Ind, m_ptr->fy, m_ptr->fx))
		{
			/* Speed up (instantly) to racial base + 10 */
			if (m_ptr->mspeed < r_ptr->speed + 10)
			{
				/* Speed up */
				m_ptr->mspeed = r_ptr->speed + 10;
				speed = TRUE;
			}
		}
	}

	/* Messages */
	if (speed) msg_print(Ind, "You feel a sudden stirring nearby!");
	else if (sleep) msg_print(Ind, "You hear a sudden stirring in the distance!");
}



/*
 * Delete all non-unique monsters of a given "type" from the level
 *
 * This is different from normal Angband now -- the closest non-unique
 * monster is chosen as the designed character to genocide.
 */
bool banishment(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int		i;

	char	typ;

	bool	result = FALSE;

	/* int		msec = delay_factor * delay_factor * delay_factor; */

	int d = 999, tmp;

	/* Hack -- Disable in dwarven halls */
	if (cfg_ironman && (!p_ptr->dun_depth % 20)) return TRUE;

	/* Search all monsters and find the closest */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];
		monster_race *r_ptr = &r_info[m_ptr->r_idx];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Hack -- Skip Unique Monsters */
		if (r_ptr->flags1 & RF1_UNIQUE) continue;

		/* Skip monsters not on this depth */
		if (p_ptr->dun_depth != m_ptr->dun_depth) continue;

		/* Check distance */
		if ((tmp = distance(p_ptr->py, p_ptr->px, m_ptr->fy, m_ptr->fx)) < d)
		{
			/* Set closest distance */
			d = tmp;

			/* Set char */
			typ = r_ptr->d_char;
		}
	}

	/* Check to make sure we found a monster */
	if (d == 999)
	{
		return FALSE;
	}

	/* Delete the monsters of that "type" */
	for (i = 1; i < m_max; i++)
	{
		monster_type	*m_ptr = &m_list[i];
		monster_race	*r_ptr = &r_info[m_ptr->r_idx];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Hack -- Skip Unique Monsters */
		if (r_ptr->flags1 & RF1_UNIQUE) continue;

		/* Skip "wrong" monsters */
		if (r_ptr->d_char != typ) continue;

		/* Skip monsters not on this depth */
		if (p_ptr->dun_depth != m_ptr->dun_depth) continue;

		/* Delete the monster */
		delete_monster_idx(i);

		/* Take damage */
		take_hit(Ind, randint(4), "the strain of casting Banishment");

		/* Redraw */
		p_ptr->redraw |= (PR_HP);

		/* Window stuff */
		/* p_ptr->window |= (PW_PLAYER); */

		/* Handle */
		handle_stuff(Ind);

		/* Fresh */
		/* Term_fresh(); */

		/* Delay */
		Send_flush(Ind);

		/* Take note */
		result = TRUE;
	}

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	/* Handle */
	handle_stuff(Ind);

	return (result);
}


/*
 * Delete all nearby (non-unique) monsters
 */
bool mass_banishment(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int		i, d;

	bool	result = FALSE;

	/*int		msec = delay_factor * delay_factor * delay_factor;*/

	/* Hack -- Disable in dwarven halls */
	if (cfg_ironman && (!p_ptr->dun_depth % 20)) return TRUE;

	/* Delete the (nearby) monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type	*m_ptr = &m_list[i];
		monster_race	*r_ptr = &r_info[m_ptr->r_idx];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip monsters not on this depth */
		if (p_ptr->dun_depth != m_ptr->dun_depth) continue;

		/* Hack -- Skip unique monsters */
		if (r_ptr->flags1 & RF1_UNIQUE) continue;

		/* Calculate the distance to this monster */
		d = distance(p_ptr->py, p_ptr->px, m_ptr->fy, m_ptr->fx);

		/* Skip distant monsters */
		if (d > MAX_SIGHT) continue;

		/* Delete the monster */
		delete_monster_idx(i);

		/* Hack -- visual feedback */
		/* does not effect the dungeon master, because it disturbs his movement
		 */
		if (strcmp(p_ptr->name,cfg_dungeon_master))
			take_hit(Ind, randint(3), "the strain of casting Mass Banishment");

		/* Redraw */
		p_ptr->redraw |= (PR_HP);

		/* Window stuff */
		/* p_ptr->window |= (PW_PLAYER); */

		/* Handle */
		handle_stuff(Ind);

		/* Fresh */
		/*Term_fresh();*/

		/* Delay */
		Send_flush(Ind);

		/* Note effect */
		result = TRUE;
	}

	/* Window stuff */
	p_ptr->window |= (PW_PLAYER);

	/* Handle */
	handle_stuff(Ind);

	return (result);
}



/*
 * Probe nearby monsters
 */
bool probing(int Ind)
{
	player_type *p_ptr = Players[Ind];
	int Depth = p_ptr->dun_depth;
	int            i,d;
	bool	probe = FALSE;

	char m_name[80];

	/* Probe all (nearby) monsters */
	for (i = 1; i < m_max; i++)
	{
		monster_type *m_ptr = &m_list[i];

		/* Paranoia -- Skip dead monsters */
		if (!m_ptr->r_idx) continue;

		/* Skip monsters not on this depth */
		if (Depth != m_ptr->dun_depth) continue;

		/* MAngband specific spell change - don't require LoS, instead limit
		 * to radius around the player */

		/* Calculate the distance to this monster */
		d = distance(p_ptr->py, p_ptr->px, m_ptr->fy, m_ptr->fx);

		/* Require line of sight */
		if (d > MAX_SIGHT) continue;

		/* Start the message */
		if (!probe) msg_print(Ind, "Probing...");

		/* Get "the monster" or "something" */
		monster_desc(Ind, m_name, i, 0x04);

		/* Describe the monster */
		msg_format(Ind, "%^s has %d hit points.", m_name, m_ptr->hp);

		/* Learn all of the non-spell, non-treasure flags */
		lore_do_probe(i);

		/* Probe worked */
		probe = TRUE;
	}

	/* Done */
	if (probe)
	{
		msg_print(Ind, "That's all.");
	}

	/* Result */
	return (probe);
}



/*
 * The spell of destruction
 *
 * This spell "deletes" monsters (instead of "killing" them).
 *
 * Later we may use one function for both "destruction" and
 * "earthquake" by using the "full" to select "destruction".
 */
void destroy_area(int Depth, int y1, int x1, int r, bool full)
{
	int y, x, k, t, Ind;

	player_type *p_ptr;

	cave_type *c_ptr;

	/*bool flag = FALSE;*/


	/* XXX XXX */
	full = full ? full : 0;

	/* Don't hurt the main town or surrounding areas */
	if (Depth <= 0 ? (wild_info[Depth].radius <= 2) : 0)
		return;

	/* Big area of affect */
	for (y = (y1 - r); y <= (y1 + r); y++)
	{
		for (x = (x1 - r); x <= (x1 + r); x++)
		{
			/* Skip illegal grids */
			if (!in_bounds(Depth, y, x)) continue;

			/* Extract the distance */
			k = distance(y1, x1, y, x);

			/* Stay in the circle of death */
			if (k > r) continue;

			/* Access the grid */
			c_ptr = &cave[Depth][y][x];

			/* Lose room and vault */
			/* Hack -- don't do this to houses/rooms outside the dungeon,
			 * this will protect hosues outside town.
			 */
			if (Depth > 0)
			{
				c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);
			}

			/* Lose light and knowledge */
			c_ptr->info &= ~(CAVE_GLOW);
			everyone_forget_spot(Depth, y, x);

			/* Hack -- Notice player affect */
			if (c_ptr->m_idx < 0)
			{
				Ind = 0 - c_ptr->m_idx;
				p_ptr = Players[Ind];

				/* Message */
				msg_print(Ind, "There is a searing blast of light!");
	
				/* Blind the player */
				if (!p_ptr->resist_blind && !p_ptr->resist_lite)
				{
					/* Become blind */
					(void)set_blind(Ind, p_ptr->blind + 10 + randint(10));
				}

				/* Mega-Hack -- Forget the view and lite */
				p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);

				/* Update stuff */
				p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);

				/* Update the monsters */
				p_ptr->update |= (PU_MONSTERS);
		
				/* Redraw map */
				p_ptr->redraw |= (PR_MAP);

				/* Window stuff */
				p_ptr->window |= (PW_OVERHEAD);

				/* Do not hurt this grid */
				continue;
			}

			/* Hack -- Skip the epicenter */
			if ((y == y1) && (x == x1)) continue;

			/* Delete the monster (if any) */
			delete_monster(Depth, y, x);

			/* Destroy "valid" grids */
			if ((cave_valid_bold(Depth, y, x)) && !(c_ptr->info&CAVE_ICKY))
			{
				/* Delete the object (if any) */
				delete_object(Depth, y, x);

				/* Wall (or floor) type */
				t = rand_int(200);

				/* Granite */
				if (t < 20)
				{
					/* Create granite wall */
					c_ptr->feat = FEAT_WALL_EXTRA;
				}

				/* Quartz */
				else if (t < 70)
				{
					/* Create quartz vein */
					c_ptr->feat = FEAT_QUARTZ;
				}

				/* Magma */
				else if (t < 100)
				{
					/* Create magma vein */
					c_ptr->feat = FEAT_MAGMA;
				}

				/* Floor */
				else
				{
					/* Create floor */
					c_ptr->feat = FEAT_FLOOR;
				}
			}
		}
	}
}


/*
 * Induce an "earthquake" of the given radius at the given location.
 *
 * This will turn some walls into floors and some floors into walls.
 *
 * The player will take damage and "jump" into a safe grid if possible,
 * otherwise, he will "tunnel" through the rubble instantaneously.
 *
 * Monsters will take damage, and "jump" into a safe grid if possible,
 * otherwise they will be "buried" in the rubble, disappearing from
 * the level in the same way that they do when genocided.
 *
 * Note that thus the player and monsters (except eaters of walls and
 * passers through walls) will never occupy the same grid as a wall.
 * Note that as of now (2.7.8) no monster may occupy a "wall" grid, even
 * for a single turn, unless that monster can pass_walls or kill_walls.
 * This has allowed massive simplification of the "monster" code.
 */
void earthquake(int Depth, int cy, int cx, int r)
{
	int		i, t, y, x, yy, xx, dy, dx, oy, ox;

	int		damage = 0;

	int		sn = 0, sy = 0, sx = 0;

	int Ind;

	player_type *p_ptr;

	/*bool	hurt = FALSE;*/

	cave_type	*c_ptr;

	bool	map[32][32];


	/* Don't hurt town or surrounding areas */
	if (Depth <= 0 ? wild_info[Depth].radius <= 2 : 0)
		return;

	/* Paranoia -- Dnforce maximum range */
	if (r > 12) r = 12;

	/* Clear the "maximal blast" area */
	for (y = 0; y < 32; y++)
	{
		for (x = 0; x < 32; x++)
		{
			map[y][x] = FALSE;
		}
	}

	/* Check around the epicenter */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip illegal grids */
			if (!in_bounds(Depth, yy, xx)) continue;

			/* Skip distant grids */
			if (distance(cy, cx, yy, xx) > r) continue;

			/* Access the grid */
			c_ptr = &cave[Depth][yy][xx];

			/* Hack -- ICKY spaces are protected outside of the dungeon */
			if ((Depth < 0) && (c_ptr->info & CAVE_ICKY)) continue;

			/* Lose room and vault */
			c_ptr->info &= ~(CAVE_ROOM | CAVE_ICKY);

			/* Lose light and knowledge */
			c_ptr->info &= ~(CAVE_GLOW);
			everyone_forget_spot(Depth, y, x);

			/* Skip the epicenter */
			if (!dx && !dy) continue;

			/* Skip most grids */
			if (rand_int(100) < 85) continue;

			/* Damage this grid */
			map[16+yy-cy][16+xx-cx] = TRUE;

			/* Hack -- Take note of player damage */
			if (c_ptr->m_idx < 0)
			{
				Ind = 0 - c_ptr->m_idx;
				p_ptr = Players[Ind];

				/* Check around the player */
				for (i = 0; i < 8; i++)
				{
					/* Access the location */
					y = p_ptr->py + ddy[i];
					x = p_ptr->px + ddx[i];

					/* Skip non-empty grids */
					if (!cave_empty_bold(Depth, y, x)) continue;

					/* Important -- Skip "quake" grids */
					if (map[16+y-cy][16+x-cx]) continue;

					/* Count "safe" grids */
					sn++;

					/* Randomize choice */
					if (rand_int(sn) > 0) continue;

					/* Save the safe location */
					sy = y; sx = x;
				}

				/* Random message */
				switch (randint(3))
				{
					case 1:
					{
						msg_print(Ind, "The cave ceiling collapses!");
						break;
					}
					case 2:
					{
						msg_print(Ind, "The cave floor twists in an unnatural way!");
						break;
					}
					default:
					{
						msg_print(Ind, "The cave quakes!  You are pummeled with debris!");
						break;
					}
				}

				/* Hurt the player a lot */
				if (!sn)
				{
					/* Message and damage */
					msg_print(Ind, "You are severely crushed!");
					damage = 300;
				}

				/* Destroy the grid, and push the player to safety */
				else
				{
					/* Calculate results */
					switch (randint(3))
					{
						case 1:
						{
							msg_print(Ind, "You nimbly dodge the blast!");
							damage = 0;
							break;
						}
						case 2:
						{
							msg_print(Ind, "You are bashed by rubble!");
							damage = damroll(10, 4);
							(void)set_stun(Ind, p_ptr->stun + randint(50));
							break;
						}
						case 3:
						{
							msg_print(Ind, "You are crushed between the floor and ceiling!");
							damage = damroll(10, 4);
							(void)set_stun(Ind, p_ptr->stun + randint(50));
							break;
						}
					}
		
					/* Save the old location */
					oy = p_ptr->py;
					ox = p_ptr->px;

					/* Move the player to the safe location */
					p_ptr->py = sy;
					p_ptr->px = sx;

					/* Update the cave player indices */
					cave[Depth][oy][ox].m_idx = 0;
					cave[Depth][sy][sx].m_idx = 0 - Ind;

					/* Redraw the old spot */
					everyone_lite_spot(Depth, oy, ox);

					/* Redraw the new spot */
					everyone_lite_spot(Depth, p_ptr->py, p_ptr->px);

					/* Check for new panel */
					verify_panel(Ind);
				}

				/* Important -- no wall on player */
				map[16+p_ptr->py-cy][16+p_ptr->px-cx] = FALSE;

				/* Take some damage */
				if (damage) take_hit(Ind, damage, "an earthquake");

				/* Mega-Hack -- Forget the view and lite */
				p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE);

				/* Update stuff */
				p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW);

				/* Update the monsters */
				p_ptr->update |= (PU_DISTANCE);

				/* Update the health bar */
				p_ptr->redraw |= (PR_HEALTH);

				/* Redraw map */
				p_ptr->redraw |= (PR_MAP);

				/* Window stuff */
				p_ptr->window |= (PW_OVERHEAD);
			}
		}
	}


	/* Examine the quaked region */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip unaffected grids */
			if (!map[16+yy-cy][16+xx-cx]) continue;

			/* Access the grid */
			c_ptr = &cave[Depth][yy][xx];

			/* Process monsters */
			if (c_ptr->m_idx > 0)
			{
				monster_type *m_ptr = &m_list[c_ptr->m_idx];
				monster_race *r_ptr = &r_info[m_ptr->r_idx];

				/* Most monsters cannot co-exist with rock */
				if (!(r_ptr->flags2 & RF2_KILL_WALL) &&
				    !(r_ptr->flags2 & RF2_PASS_WALL))
				{
					/* Assume not safe */
					sn = 0;

					/* Monster can move to escape the wall */
					if (!(r_ptr->flags1 & RF1_NEVER_MOVE))
					{
						/* Look for safety */
						for (i = 0; i < 8; i++)
						{
							/* Access the grid */
							y = yy + ddy[i];
							x = xx + ddx[i];

							/* Skip non-empty grids */
							if (!cave_empty_bold(Depth, y, x)) continue;

							/* Hack -- no safety on glyph of warding */
							if (cave[Depth][y][x].feat == FEAT_GLYPH) continue;

							/* Important -- Skip "quake" grids */
							if (map[16+y-cy][16+x-cx]) continue;

							/* Count "safe" grids */
							sn++;

							/* Randomize choice */
							if (rand_int(sn) > 0) continue;

							/* Save the safe grid */
							sy = y; sx = x;
						}
					}

					/* Describe the monster */
					/*monster_desc(Ind, m_name, c_ptr->m_idx, 0);*/

					/* Scream in pain */
					/*msg_format("%^s wails out in pain!", m_name);*/

					/* Take damage from the quake */
					damage = (sn ? damroll(4, 8) : (m_ptr->hp + 1));

					/* Monster is certainly awake */
					m_ptr->csleep = 0;

					/* Apply damage directly */
					m_ptr->hp -= damage;

					/* Delete (not kill) "dead" monsters */
					if (m_ptr->hp < 0)
					{
						/* Message */
						/*msg_format("%^s is embedded in the rock!", m_name);*/

						/* Delete the monster */
						delete_monster(Depth, yy, xx);

						/* No longer safe */
						sn = 0;
					}

					/* Hack -- Escape from the rock */
					if (sn)
					{
						int m_idx = cave[Depth][yy][xx].m_idx;

						/* Update the new location */
						cave[Depth][sy][sx].m_idx = m_idx;

						/* Update the old location */
						cave[Depth][yy][xx].m_idx = 0;

						/* Move the monster */
						m_ptr->fy = sy;
						m_ptr->fx = sx;

						/* Update the monster (new location) */
						update_mon(m_idx, TRUE);

						/* Redraw the old grid */
						everyone_lite_spot(Depth, yy, xx);

						/* Redraw the new grid */
						everyone_lite_spot(Depth, sy, sx);
					}
				}
			}
		}
	}


	/* Examine the quaked region */
	for (dy = -r; dy <= r; dy++)
	{
		for (dx = -r; dx <= r; dx++)
		{
			/* Extract the location */
			yy = cy + dy;
			xx = cx + dx;

			/* Skip unaffected grids */
			if (!map[16+yy-cy][16+xx-cx]) continue;

			/* Access the cave grid */
			c_ptr = &cave[Depth][yy][xx];

			/* Paranoia -- never affect player */
			if (c_ptr->m_idx < 0) continue;

			/* Destroy location (if valid) */
			if (cave_valid_bold(Depth, yy, xx))
			{
				bool floor = cave_floor_bold(Depth, yy, xx);

				/* Delete any object that is still there */
				delete_object(Depth, yy, xx);

				/* Wall (or floor) type */
				t = (floor ? rand_int(100) : 200);

				/* Granite */
				if (t < 20)
				{
					/* Create granite wall */
					c_ptr->feat = FEAT_WALL_EXTRA;
				}

				/* Quartz */
				else if (t < 70)
				{
					/* Create quartz vein */
					c_ptr->feat = FEAT_QUARTZ;
				}

				/* Magma */
				else if (t < 100)
				{
					/* Create magma vein */
					c_ptr->feat = FEAT_MAGMA;
				}

				/* Floor */
				else
				{
					/* Create floor */
					c_ptr->feat = FEAT_FLOOR;
				}
			}
		}
	}
}



/*
 * This routine clears the entire "temp" set.
 *
 * This routine will Perma-Lite all "temp" grids.
 *
 * This routine is used (only) by "lite_room()"
 *
 * Dark grids are illuminated.
 *
 * Also, process all affected monsters.
 *
 * SMART monsters always wake up when illuminated
 * NORMAL monsters wake up 1/4 the time when illuminated
 * STUPID monsters wake up 1/10 the time when illuminated
 */
static void cave_temp_room_lite(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int Depth = p_ptr->dun_depth;

	int i;

	/* Clear them all */
	for (i = 0; i < p_ptr->temp_n; i++)
	{
		int y = p_ptr->temp_y[i];
		int x = p_ptr->temp_x[i];

		cave_type *c_ptr = &cave[Depth][y][x];

		/* No longer in the array */
		c_ptr->info &= ~CAVE_TEMP;

		/* Update only non-CAVE_GLOW grids */
		/* if (c_ptr->info & CAVE_GLOW) continue; */

		/* Perma-Lite */
		c_ptr->info |= CAVE_GLOW;

		/* Process affected monsters */
		if (c_ptr->m_idx > 0)
		{
			int chance = 25;

			monster_type	*m_ptr = &m_list[c_ptr->m_idx];

			monster_race	*r_ptr = &r_info[m_ptr->r_idx];

			/* Update the monster */
			update_mon(c_ptr->m_idx, FALSE);

			/* Stupid monsters rarely wake up */
			if (r_ptr->flags2 & RF2_STUPID) chance = 10;

			/* Smart monsters always wake up */
			if (r_ptr->flags2 & RF2_SMART) chance = 100;

			/* Sometimes monsters wake up */
			if (m_ptr->csleep && (rand_int(100) < chance))
			{
				/* Wake up! */
				m_ptr->csleep = 0;

				/* Notice the "waking up" */
				if (p_ptr->mon_vis[c_ptr->m_idx])
				{
					char m_name[80];

					/* Acquire the monster name */
					monster_desc(Ind, m_name, c_ptr->m_idx, 0);

					/* Dump a message */
					msg_format(Ind, "%^s wakes up.", m_name);
				}
			}
		}

		/* Note */
		note_spot_depth(Depth, y, x);

		/* Redraw */
		everyone_lite_spot(Depth, y, x);
	}

	/* None left */
	p_ptr->temp_n = 0;
}



/*
 * This routine clears the entire "temp" set.
 *
 * This routine will "darken" all "temp" grids.
 *
 * In addition, some of these grids will be "unmarked".
 *
 * This routine is used (only) by "unlite_room()"
 *
 * Also, process all affected monsters
 */
static void cave_temp_room_unlite(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int Depth = p_ptr->dun_depth;

	int i;

	/* Clear them all */
	for (i = 0; i < p_ptr->temp_n; i++)
	{
		int y = p_ptr->temp_y[i];
		int x = p_ptr->temp_x[i];

		cave_type *c_ptr = &cave[Depth][y][x];

		/* No longer in the array */
		c_ptr->info &= ~CAVE_TEMP;

		/* Darken the grid */
		c_ptr->info &= ~CAVE_GLOW;

		/* Hack -- Forget "boring" grids */
		if (is_boring(c_ptr->feat))
		{
			/* Forget the grid */
			p_ptr->cave_flag[y][x] &= ~CAVE_MARK;

			/* Notice */
			note_spot_depth(Depth, y, x);
		}

		/* Process affected monsters */
		if (c_ptr->m_idx > 0)
		{
			/* Update the monster */
			update_mon(c_ptr->m_idx, FALSE);
		}

		/* Redraw */
		everyone_lite_spot(Depth, y, x);
	}

	/* None left */
	p_ptr->temp_n = 0;
}




/*
 * Aux function -- see below
 */
static void cave_temp_room_aux(int Ind, int Depth, int y, int x)
{
	player_type *p_ptr = Players[Ind];

	cave_type *c_ptr = &cave[Depth][y][x];

	/* Avoid infinite recursion */
	if (c_ptr->info & CAVE_TEMP) return;

	/* Do not "leave" the current room */
	if (!(c_ptr->info & CAVE_ROOM)) return;

	/* Paranoia -- verify space */
	if (p_ptr->temp_n == TEMP_MAX) return;

	/* Mark the grid as "seen" */
	c_ptr->info |= CAVE_TEMP;

	/* Add it to the "seen" set */
	p_ptr->temp_y[p_ptr->temp_n] = y;
	p_ptr->temp_x[p_ptr->temp_n] = x;
	p_ptr->temp_n++;
}




/*
 * Illuminate any room containing the given location.
 */
void lite_room(int Ind, int Depth, int y1, int x1)
{
	player_type *p_ptr = Players[Ind];

	int i, x, y;

	/* Add the initial grid */
	cave_temp_room_aux(Ind, Depth, y1, x1);

	/* While grids are in the queue, add their neighbors */
	for (i = 0; i < p_ptr->temp_n; i++)
	{
		x = p_ptr->temp_x[i], y = p_ptr->temp_y[i];

		/* Walls get lit, but stop light */
		if (!cave_floor_bold(Depth, y, x)) continue;

		/* Spread adjacent */
		cave_temp_room_aux(Ind, Depth, y + 1, x);
		cave_temp_room_aux(Ind, Depth, y - 1, x);
		cave_temp_room_aux(Ind, Depth, y, x + 1);
		cave_temp_room_aux(Ind, Depth, y, x - 1);

		/* Spread diagonal */
		cave_temp_room_aux(Ind, Depth, y + 1, x + 1);
		cave_temp_room_aux(Ind, Depth, y - 1, x - 1);
		cave_temp_room_aux(Ind, Depth, y - 1, x + 1);
		cave_temp_room_aux(Ind, Depth, y + 1, x - 1);
	}

	/* Now, lite them all up at once */
	cave_temp_room_lite(Ind);
}


/*
 * Darken all rooms containing the given location
 */
void unlite_room(int Ind, int Depth, int y1, int x1)
{
	player_type *p_ptr = Players[Ind];

	int i, x, y;

	/* Add the initial grid */
	cave_temp_room_aux(Ind, Depth, y1, x1);

	/* Spread, breadth first */
	for (i = 0; i < p_ptr->temp_n; i++)
	{
		x = p_ptr->temp_x[i], y = p_ptr->temp_y[i];

		/* Walls get dark, but stop darkness */
		if (!cave_floor_bold(Depth, y, x)) continue;

		/* Spread adjacent */
		cave_temp_room_aux(Ind, Depth, y + 1, x);
		cave_temp_room_aux(Ind, Depth, y - 1, x);
		cave_temp_room_aux(Ind, Depth, y, x + 1);
		cave_temp_room_aux(Ind, Depth, y, x - 1);

		/* Spread diagonal */
		cave_temp_room_aux(Ind, Depth, y + 1, x + 1);
		cave_temp_room_aux(Ind, Depth, y - 1, x - 1);
		cave_temp_room_aux(Ind, Depth, y - 1, x + 1);
		cave_temp_room_aux(Ind, Depth, y + 1, x - 1);
	}

	/* Now, darken them all at once */
	cave_temp_room_unlite(Ind);
}



/*
 * Hack -- call light around the player
 * Affect all monsters in the projection radius
 */
bool lite_area(int Ind, int dam, int rad)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_GRID | PROJECT_KILL;

	/* Hack -- Message */
	if (!p_ptr->blind)
	{
		msg_print(Ind, "You are surrounded by a white light.");
	}

	/* Hook into the "project()" function */
	(void)project(0 - Ind, rad, p_ptr->dun_depth, p_ptr->py, p_ptr->px, dam, GF_LITE_WEAK, flg);

	/* Lite up the room */
	lite_room(Ind, p_ptr->dun_depth, p_ptr->py, p_ptr->px);

	/* Assume seen */
	return (TRUE);
}


/*
 * Hack -- call darkness around the player
 * Affect all monsters in the projection radius
 */
bool unlite_area(int Ind, int dam, int rad)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_GRID | PROJECT_KILL;

	/* Hack -- Message */
	if (!p_ptr->blind)
	{
		msg_print(Ind, "Darkness surrounds you.");
	}

	/* Hook into the "project()" function */
	(void)project(0 - Ind, rad, p_ptr->dun_depth, p_ptr->py, p_ptr->px, dam, GF_DARK_WEAK, flg);

	/* Lite up the room */
	unlite_room(Ind, p_ptr->dun_depth, p_ptr->py, p_ptr->px);

	/* Assume seen */
	return (TRUE);
}



/*
 * Cast a ball spell
 * Stop if we hit a monster, act as a "ball"
 * Allow "target" mode to pass over monsters
 * Affect grids, objects, and monsters
 */
bool fire_ball(int Ind, int typ, int dir, int dam, int rad)
{
	player_type *p_ptr = Players[Ind];

	int tx, ty;

	int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

  if (dir < 0 || dir > 10) return FALSE;

	/* Use the given direction */
	tx = p_ptr->px + 99 * ddx[dir];
	ty = p_ptr->py + 99 * ddy[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay(Ind))
	{
		flg &= ~PROJECT_STOP;
		tx = p_ptr->target_col;
		ty = p_ptr->target_row;
	}

	/* Analyze the "dir" and the "target".  Hurt items on floor. */
	return (project(0 - Ind, rad, p_ptr->dun_depth, ty, tx, dam, typ, flg));
}

/*
 * Cast multiple non-jumping ball spells at the same target.
 *
 * Targets absolute coordinates instead of a specific monster, so that
 * the death of the monster doesn't change the target's location.
 */
bool fire_swarm(int Ind, int num, int typ, int dir, int dam, int rad)
{
	player_type *p_ptr = Players[Ind];
	bool noticed = FALSE;

	int py = p_ptr->py;
	int px = p_ptr->px;

	int ty, tx;

	int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

  if (dir < 0 || dir > 10) return FALSE;

	/* Use the given direction */
	ty = py + 99 * ddy[dir];
	tx = px + 99 * ddx[dir];

	/* Hack -- Use an actual "target" (early detonation) */
	if ((dir == 5) && target_okay(Ind))
	{
		ty = p_ptr->target_row;
		tx = p_ptr->target_col;
	}

	while (num--)
	{
		/* Analyze the "dir" and the "target".  Hurt items on floor. */
		if (project(0 - Ind, rad, p_ptr->dun_depth, ty, tx, dam, typ, flg)) noticed = TRUE;
	}

	return noticed;
}



/*
 * Hack -- apply a "projection()" in a direction (or at the target)
 */
bool project_hook(int Ind, int typ, int dir, int dam, int flg)
{
	player_type *p_ptr = Players[Ind];

	int tx, ty;

	/* Pass through the target if needed */
	flg |= (PROJECT_THRU);

  if (dir < 0 || dir > 10) return FALSE;

	/* Use the given direction */
	tx = p_ptr->px + ddx[dir];
	ty = p_ptr->py + ddy[dir];

	/* Hack -- Use an actual "target" */
	if ((dir == 5) && target_okay(Ind))
	{
		tx = p_ptr->target_col;
		ty = p_ptr->target_row;
	}

	/* Analyze the "dir" and the "target", do NOT explode */
	return (project(0 - Ind, 0, p_ptr->dun_depth, ty, tx, dam, typ, flg));
}


/*
 * Cast a bolt spell
 * Stop if we hit a monster, as a "bolt"
 * Affect monsters (not grids or objects)
 */
bool fire_bolt(int Ind, int typ, int dir, int dam)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, typ, dir, dam, flg));
}

/*
 * Cast a beam spell
 * Pass through monsters, as a "beam"
 * Affect monsters (not grids or objects)
 */
bool fire_beam(int Ind, int typ, int dir, int dam)
{
	int flg = PROJECT_BEAM | PROJECT_KILL;
	return (project_hook(Ind, typ, dir, dam, flg));
}

/*
 * Cast a bolt spell, or rarely, a beam spell
 */
bool fire_bolt_or_beam(int Ind, int prob, int typ, int dir, int dam)
{
	if (rand_int(100) < prob)
	{
		return (fire_beam(Ind, typ, dir, dam));
	}
	else
	{
		return (fire_bolt(Ind, typ, dir, dam));
	}
}


/*
 * Some of the old functions
 */

bool lite_line(int Ind, int dir)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL;
	return (project_hook(Ind, GF_LITE_WEAK, dir, damroll(6, 8), flg));
}

bool strong_lite_line(int Ind, int dir)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_KILL;
	return (project_hook(Ind, GF_LITE, dir, damroll(10, 8), flg));
}

bool drain_life(int Ind, int dir, int dam)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_OLD_DRAIN, dir, dam, flg));
}

bool wall_to_mud(int Ind, int dir)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
	return (project_hook(Ind, GF_KILL_WALL, dir, 20 + randint(30), flg));
}

bool destroy_door(int Ind, int dir)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM;
	return (project_hook(Ind, GF_KILL_DOOR, dir, 0, flg));
}

bool disarm_trap(int Ind, int dir)
{
	int flg = PROJECT_BEAM | PROJECT_GRID | PROJECT_ITEM;
	return (project_hook(Ind, GF_KILL_TRAP, dir, 0, flg));
}

bool heal_monster(int Ind, int dir)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_OLD_HEAL, dir, damroll(4, 6), flg));
}

bool speed_monster(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_OLD_SPEED, dir, p_ptr->lev, flg));
}

bool slow_monster(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_OLD_SLOW, dir, p_ptr->lev, flg));
}

bool sleep_monster(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_OLD_SLEEP, dir, p_ptr->lev, flg));
}

bool confuse_monster(int Ind, int dir, int plev)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_OLD_CONF, dir, plev, flg));
}

bool poly_monster(int Ind, int dir)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_STOP | PROJECT_KILL;

	/* Never in the town */
	if(!p_ptr->dun_depth) return(FALSE);

	return (project_hook(Ind, GF_OLD_POLY, dir, p_ptr->lev, flg));
}

bool clone_monster(int Ind, int dir)
{
	int num, i;
	player_type *p_ptr = Players[Ind];

	/* Never in the town */
	if(!p_ptr->dun_depth) return(FALSE);
	
	/* Restricted in MAngband. This behaviour is not in vanilla Angband, in vanilla however,
	 * cloning 100 Great Hell Wyrms effects only the cloner. In MAngband this effects all 
	 * players and the whole game economy as the game is flooded with great items */
	if(p_ptr->lev >= 10)
	{
		msg_print(Ind, "The wand crackles loudly. Nothing else happens.");
		return(TRUE);
	}
	else
	{
		/* Standard clone effect */
		int flg = PROJECT_STOP | PROJECT_KILL;
		return (project_hook(Ind, GF_OLD_CLONE, dir, 0, flg));
	}
}

bool fear_monster(int Ind, int dir, int plev)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_TURN_ALL, dir, plev, flg));
}

bool teleport_monster(int Ind, int dir)
{
	int flg = PROJECT_BEAM | PROJECT_KILL;
	return (project_hook(Ind, GF_AWAY_ALL, dir, MAX_SIGHT * 5, flg));
}

bool cure_light_wounds_proj(int Ind, int dir)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_HEAL_PLAYER, dir, damroll(2, 10), flg));
}

bool cure_serious_wounds_proj(int Ind, int dir)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_HEAL_PLAYER, dir, damroll(4, 10), flg));
}

bool cure_critical_wounds_proj(int Ind, int dir)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_HEAL_PLAYER, dir, damroll(6, 10), flg));
}

bool cure_mortal_wounds_proj(int Ind, int dir)
{
    int flg = PROJECT_STOP | PROJECT_KILL;
    return (project_hook(Ind, GF_HEAL_PLAYER, dir, damroll(8, 10), flg));
}

bool heal_other_lite_proj(int Ind, int dir)
{
    int flg = PROJECT_STOP | PROJECT_KILL;
    return (project_hook(Ind, GF_HEAL_PLAYER, dir, 100, flg));
}

bool heal_other_proj(int Ind, int dir)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_HEAL_PLAYER, dir, 300, flg));
}

bool heal_other_heavy_proj(int Ind, int dir)
{
	int flg = PROJECT_STOP | PROJECT_KILL;
	return (project_hook(Ind, GF_HEAL_PLAYER, dir, 2000, flg));
}



/*
 * Hooks -- affect adjacent grids (radius 1 ball attack)
 */

bool door_creation(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
	if(p_ptr->dun_depth == 0 ) { return 0;};
	return (project(0 - Ind, 1, p_ptr->dun_depth, p_ptr->py, p_ptr->px, 0, GF_MAKE_DOOR, flg));
}

bool trap_creation(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
	if(p_ptr->dun_depth == 0 ) { return 0;};
	return (project(0 - Ind, 1, p_ptr->dun_depth, p_ptr->py, p_ptr->px, 0, GF_MAKE_TRAP, flg));
}

bool destroy_doors_touch(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_HIDE;
	return (project(0 - Ind, 1, p_ptr->dun_depth, p_ptr->py, p_ptr->px, 0, GF_KILL_DOOR, flg));
}

bool sleep_monsters_touch(int Ind)
{
	player_type *p_ptr = Players[Ind];

	int flg = PROJECT_KILL | PROJECT_HIDE;
	return (project(0 - Ind, 1, p_ptr->dun_depth, p_ptr->py, p_ptr->px, p_ptr->lev, GF_OLD_SLEEP, flg));
}

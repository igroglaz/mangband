/* File: melee1.c */

/* Purpose: Monster attacks */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "mangband.h"

/* Send "slash effect" to the client */
void slash_fx(int Ind, int agressor, int victim, u16b sound_msg)
{
	player_type *p_ptr = Players[Ind];
	int x1, y1, x2, y2, dx, dy;
	char dir;
	const char dirrrs[3][3] = {
		{ 7, 8, 9 },
		{ 4, 5, 6 },
		{ 1, 2, 3 },
	};
	player_type *q_ptr, *q2_ptr;
	monster_type *m_ptr, *m2_ptr;
	if (victim < 0)
	{
		m_ptr = &m_list[0 - victim];
		x1 = m_ptr->fx;
		y1 = m_ptr->fy;
	}
	else
	{
		q_ptr = Players[victim];
		x1 = q_ptr->px;
		y1 = q_ptr->py;
	}
	if (agressor < 0)
	{
		m2_ptr = &m_list[0 - agressor];
		x2 = m2_ptr->fx;
		y2 = m2_ptr->fy;
	}
	else
	{
		q2_ptr = Players[agressor];
		x2 = q2_ptr->px;
		y2 = q2_ptr->py;
	}
	/* Determine direction */
	dx = x2 - x1;
	dy = y2 - y1;
	if (dx < -1) dx = -1;
	if (dx > 1) dx = 1;
	if (dy < -1) dy = -1;
	if (dy > 1) dy = 1;
	dir = dirrrs[dy + 1][dx + 1];
	/* Send it ! */
	send_slash_fx(Ind, y2 - p_ptr->panel_row_min, x2 - p_ptr->panel_col_min, dir, sound_msg);
}

/*
 * Critical blow.  All hits that do 95% of total possible damage,
 * and which also do at least 20 damage, or, sometimes, N damage.
 * This is used only to determine "cuts" and "stuns".
 */
static int monster_critical(int dice, int sides, int dam)
{
	int max = 0;
	int total = dice * sides;

	/* Must do at least 95% of perfect */
	if (dam < total * 19 / 20) return (0);

	/* Weak blows rarely work */
	if ((dam < 20) && (rand_int(100) >= dam)) return (0);

	/* Perfect damage */
	if (dam == total) max++;

	/* Super-charge */
	if (dam >= 20)
	{
		while (rand_int(100) < 2) max++;
	}

	/* Critical damage */
	if (dam > 45) return (6 + max);
	if (dam > 33) return (5 + max);
	if (dam > 25) return (4 + max);
	if (dam > 18) return (3 + max);
	if (dam > 11) return (2 + max);
	return (1 + max);
}





/*
 * Determine if a monster attack against the player succeeds.
 * Always miss 5% of the time, Always hit 5% of the time.
 * Otherwise, match monster power against player armor.
 */
static int check_hit(int Ind, int power, int level)
{
	player_type *p_ptr = Players[Ind];

	int i, k, ac;

	/* Percentile dice */
	k = rand_int(100);

	/* Hack -- Always miss or hit */
	if (k < 10) return (k < 5);

	/* Calculate the "attack quality" */
	i = (power + (level * 3));

	/* Total armor */
	ac = p_ptr->ac + p_ptr->to_a;

	/* Power and Level compete against Armor */
	if ((i > 0) && (randint(i) > ((ac * 3) / 4))) return (TRUE);

	/* Assume miss */
	return (FALSE);
}



/*
 * Hack -- possible "insult" messages
 */
static cptr desc_insult[] =
{
	"insults you!",
	"insults your mother!",
	"gives you the finger!",
	"humiliates you!",
	"defiles you!",
	"dances around you!",
	"makes obscene gestures!",
	"moons you!!!"
};



/*
 * Hack -- possible "insult" messages
 */
static cptr desc_moan[] =
{
	"seems sad about something.",
	"asks if you have seen his dogs.",
	"tells you to get off his land.",
	"mumbles something about mushrooms."
};


/*
 * Attack a player via physical attacks.
 */
bool make_attack_normal(int Ind, int m_idx)
{
	player_type *p_ptr = Players[Ind];

	monster_type	*m_ptr = &m_list[m_idx];

	monster_race	*r_ptr = &r_info[m_ptr->r_idx];
	
	monster_lore	*l_ptr = p_ptr->l_list + m_ptr->r_idx;

	int			ap_cnt;

	int			i, j, k, tmp, ac, rlev;
	int			do_cut, do_stun;

	s32b		gold;

	object_type		*o_ptr;

	char		o_name[80];

	char		m_name[80];

	char		ddesc[80];

	int		blinked;
	int		sound_msg;


	/* Not allowed to attack */
	if (r_ptr->flags1 & RF1_NEVER_BLOW) return (FALSE);

	/* Total armor */
	ac = p_ptr->ac + p_ptr->to_a;

	/* Extract the effective monster level */
	rlev = ((r_ptr->level >= 1) ? r_ptr->level : 1);


	/* Get the monster name (or "it") */
	monster_desc(Ind, m_name, m_idx, 0);

	/* Get the "died from" information (i.e. "a kobold") */
	monster_desc(Ind, ddesc, m_idx, 0x88);


	/* Assume no blink */
	blinked = 0;

	/* Scan through all four blows */
	for (ap_cnt = 0; ap_cnt < 4; ap_cnt++)
	{
		bool visible = FALSE;
		bool obvious = FALSE;

		int power = 0;
		int damage = 0;

		cptr act = NULL;

		/* Extract the attack infomation */
		int effect = r_ptr->blow[ap_cnt].effect;
		int method = r_ptr->blow[ap_cnt].method;
		int d_dice = r_ptr->blow[ap_cnt].d_dice;
		int d_side = r_ptr->blow[ap_cnt].d_side;


		/* Hack -- no more attacks */
		if (!method) break;

		/* Stop if player is dead or gone */
		if (!p_ptr->alive || p_ptr->death || p_ptr->new_level_flag) break;


		/* Extract visibility (before blink) */
		if (p_ptr->mon_vis[m_idx]) visible = TRUE;



		/* Extract the attack "power" */
		switch (effect)
		{
			case RBE_HURT:	power = 60; break;
			case RBE_POISON:	power =  5; break;
			case RBE_UN_BONUS:	power = 20; break;
			case RBE_UN_POWER:	power = 15; break;
			case RBE_EAT_GOLD:	power =  5; break;
			case RBE_EAT_ITEM:	power =  5; break;
			case RBE_EAT_FOOD:	power =  5; break;
			case RBE_EAT_LITE:	power =  5; break;
			case RBE_ACID:	power =  0; break;
			case RBE_ELEC:	power = 10; break;
			case RBE_FIRE:	power = 10; break;
			case RBE_COLD:	power = 10; break;
			case RBE_BLIND:	power =  2; break;
			case RBE_CONFUSE:	power = 10; break;
			case RBE_TERRIFY:	power = 10; break;
			case RBE_PARALYZE:	power =  2; break;
			case RBE_LOSE_STR:	power =  0; break;
			case RBE_LOSE_DEX:	power =  0; break;
			case RBE_LOSE_CON:	power =  0; break;
			case RBE_LOSE_INT:	power =  0; break;
			case RBE_LOSE_WIS:	power =  0; break;
			case RBE_LOSE_CHR:	power =  0; break;
			case RBE_LOSE_ALL:	power =  2; break;
			case RBE_SHATTER:	power = 60; break;
			case RBE_EXP_10:	power =  5; break;
			case RBE_EXP_20:	power =  5; break;
			case RBE_EXP_40:	power =  5; break;
			case RBE_EXP_80:	power =  5; break;
			case RBE_HALLU:     power = 10; break;
		}


		/* Monster hits player */
		if (!effect || check_hit(Ind, power, rlev))
		{
			/* Always disturbing */
			disturb(Ind, 1, 0);


			/* Hack -- Apply "protection from evil" */
			if ((p_ptr->protevil > 0) &&
			    (r_ptr->flags3 & RF3_EVIL) &&
			    (p_ptr->lev >= rlev) &&
			    ((rand_int(100) + p_ptr->lev) > 50))
			{
				/* Remember the Evil-ness */
				if (p_ptr->mon_vis[m_idx]) l_ptr->flags3 |= RF3_EVIL;

				/* Message */
				msg_format(Ind, "%^s is repelled.", m_name);

				/* Hack -- Next attack */
				continue;
			}


			/* Assume no cut or stun */
			do_cut = do_stun = 0;

			/* Assume no sound */
			sound_msg = MSG_GENERIC;

			/* Describe the attack method */
			switch (method)
			{
				case RBM_HIT:
				{
					act = "hits you.";
					do_cut = do_stun = 1;
					sound_msg = MSG_MON_HIT;
					break;
				}

				case RBM_TOUCH:
				{
					act = "touches you.";
					sound_msg = MSG_MON_TOUCH;
					break;
				}

				case RBM_PUNCH:
				{
					act = "punches you.";
					do_stun = 1;
					sound_msg = MSG_MON_PUNCH;
					break;
				}

				case RBM_KICK:
				{
					act = "kicks you.";
					do_stun = 1;
					sound_msg = MSG_MON_KICK;
					break;
				}

				case RBM_CLAW:
				{
					act = "claws you.";
					do_cut = 1;
					sound_msg = MSG_MON_CLAW;
					break;
				}

				case RBM_BITE:
				{
					act = "bites you.";
					do_cut = 1;
					sound_msg = MSG_MON_BITE;
					break;
				}

				case RBM_STING:
				{
					act = "stings you.";
					sound_msg = MSG_MON_STING;
					break;
				}

				case RBM_XXX1:
				{
					act = "XXX1's you.";
					break;
				}

				case RBM_BUTT:
				{
					act = "butts you.";
					do_stun = 1;
					sound_msg = MSG_MON_BUTT;
					break;
				}

				case RBM_CRUSH:
				{
					act = "crushes you.";
					do_stun = 1;
					sound_msg = MSG_MON_CRUSH;
					break;
				}

				case RBM_ENGULF:
				{
					act = "engulfs you.";
					sound_msg = MSG_MON_ENGULF;
					break;
				}

				case RBM_XXX2:
				{
					act = "XXX2's you.";
					break;
				}

				case RBM_CRAWL:
				{
					act = "crawls on you.";
					sound_msg = MSG_MON_CRAWL;
					break;
				}

				case RBM_DROOL:
				{
					act = "drools on you.";
					sound_msg = MSG_MON_DROOL;
					break;
				}

				case RBM_SPIT:
				{
					act = "spits on you.";
					sound_msg = MSG_MON_SPIT;
					break;
				}

				case RBM_XXX3:
				{
					act = "XXX3's on you.";
					break;
				}

				case RBM_GAZE:
				{
					act = "gazes at you.";
					sound_msg = MSG_MON_GAZE;
					break;
				}

				case RBM_WAIL:
				{
					act = "wails at you.";
					sound_msg = MSG_MON_WAIL;
					break;
				}

				case RBM_SPORE:
				{
					act = "releases spores at you.";
					sound_msg = MSG_MON_SPORE;
					break;
				}

				case RBM_XXX4:
				{
					act = "projects XXX4's at you.";
					break;
				}

				case RBM_BEG:
				{
					act = "begs you for money.";
					sound_msg = MSG_MON_BEG;
					break;
				}

				case RBM_INSULT:
				{
					act = desc_insult[rand_int(8)];
					sound_msg = MSG_MON_INSULT;
					break;
				}

				case RBM_MOAN:
				{
					act = desc_moan[rand_int(4)];
					sound_msg = MSG_MON_MOAN;
					break;
				}

				case RBM_XXX5:
				{
					act = "XXX5's you.";
					break;
				}
			}

			/* Message */
			if (act) msg_format(Ind, "%^s %s", m_name, act);
			if (act) sound(Ind, sound_msg);
			if (act) slash_fx(Ind, 0 - m_idx, Ind, sound_msg);

			/* Hack -- assume all attacks are obvious */
			obvious = TRUE;

			/* Roll out the damage */
			damage = damroll(d_dice, d_side);

			/* Apply appropriate damage */
			switch (effect)
			{
				case 0:
				{
					/* Hack -- Assume obvious */
					obvious = TRUE;

					/* Hack -- No damage */
					damage = 0;

					break;
				}

				case RBE_HURT:
				{
					/* Obvious */
					obvious = TRUE;

					/* Hack -- Player armor reduces total damage */
					damage -= (damage * ((ac < 150) ? ac : 150) / 250);

					/* Take damage */
					take_hit(Ind, damage, ddesc);

					break;
				}

				case RBE_POISON:
				{
					/* Take some damage */
					take_hit(Ind, damage, ddesc);

					/* Take "poison" effect */
					if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
					{
						if (set_poisoned(Ind, p_ptr->poisoned + randint(rlev) + 5))
						{
							obvious = TRUE;
						}
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_POIS);

					break;
				}

				case RBE_UN_BONUS:
				{
					/* Take some damage */
					take_hit(Ind, damage, ddesc);

					/* Allow complete resist */
					if (!p_ptr->resist_disen)
					{
						/* Apply disenchantment */
						if (apply_disenchant(Ind, 0)) obvious = TRUE;
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_DISEN);

					break;
				}

				case RBE_UN_POWER:
				{
					/* Take some damage */
					take_hit(Ind, damage, ddesc);

					/* Find an item */
					for (k = 0; k < 10; k++)
					{
						/* Pick an item */
						i = rand_int(INVEN_PACK);

						/* Obtain the item */
						o_ptr = &p_ptr->inventory[i];

						/* Skip non-objects */
						if (!o_ptr->k_idx) continue;

						/* Drain charged wands/staffs */
						if (((o_ptr->tval == TV_STAFF) ||
						     (o_ptr->tval == TV_WAND)) &&
						    (o_ptr->pval))
						{
							/* Message */
							msg_print(Ind, "Energy drains from your pack!");

							/* Obvious */
							obvious = TRUE;

							/* Heal */
							j = rlev;
							m_ptr->hp += j * o_ptr->pval;
							if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

							/* Redraw (later) if needed */
							update_health(m_idx);

							/* Uncharge */
							o_ptr->pval = 0;

							/* Combine / Reorder the pack */
							p_ptr->notice |= (PN_COMBINE | PN_REORDER);

							/* Window stuff */
							p_ptr->window |= (PW_INVEN);

							/* Done */
							break;
						}
					}

					break;
				}

				case RBE_EAT_GOLD:
				{
					/* Take some damage */
					take_hit(Ind, damage, ddesc);

					/* Obvious */
					obvious = TRUE;

					/* Saving throw (unless paralyzed) based on dex and level */
					if (!p_ptr->paralyzed &&
					    (rand_int(100) < (adj_dex_safe[p_ptr->stat_ind[A_DEX]] +
					                      p_ptr->lev)))
					{
						/* Saving throw message */
						msg_print(Ind, "You quickly protect your money pouch!");

						/* Occasional blink anyway */
						if (rand_int(3)) blinked = 2;
					}

					/* Eat gold */
					else
					{
						gold = (p_ptr->au / 10) + randint(25);
						if (gold < 2) gold = 2;
						if (gold > 5000) gold = (p_ptr->au / 20) + randint(3000);
						if (gold > p_ptr->au) gold = p_ptr->au;
						p_ptr->au -= gold;
						if (gold <= 0)
						{
							msg_print(Ind, "Nothing was stolen.");
						}
						else if (p_ptr->au)
						{
							msg_print(Ind, "Your purse feels lighter.");
							msg_format(Ind, "%ld coins were stolen!", (long)gold);
						}
						else
						{
							msg_print(Ind, "Your purse feels lighter.");
							msg_print(Ind, "All of your coins were stolen!");
						}

						/* Redraw gold */
						p_ptr->redraw |= (PR_GOLD);

						/* Window stuff */
						p_ptr->window |= (PW_PLAYER);

						/* Blink away */
						blinked = 2;
					}

					break;
				}

				case RBE_EAT_ITEM:
				{
					/* Take some damage */
					take_hit(Ind, damage, ddesc);

					/* Saving throw (unless paralyzed) based on dex and level */
					if (!p_ptr->paralyzed &&
					    (rand_int(100) < (adj_dex_safe[p_ptr->stat_ind[A_DEX]] +
					                      p_ptr->lev)))
					{
						/* Saving throw message */
						msg_print(Ind, "You grab hold of your backpack!");

						/* Occasional "blink" anyway */
						blinked = 2;

						/* Obvious */
						obvious = TRUE;

						/* Done */
						break;
					}

					/* Find an item */
					for (k = 0; k < 10; k++)
					{
						/* Item copy */
 						object_type *i_ptr; 
						object_type object_type_body;					
					
						/* Pick an item */
						i = rand_int(INVEN_PACK);

						/* Obtain the item */
						o_ptr = &p_ptr->inventory[i];

						/* Accept real items */
						if (!o_ptr->k_idx) continue;

						/* Don't steal artifacts  -CFT */
						if (artifact_p(o_ptr)) continue;

						/* Get local object */ 
						i_ptr = &object_type_body; 
	
						/* Obtain local object */ 
						COPY(i_ptr, o_ptr, object_type); 
	
						/* Modify number */ 
						i_ptr->number = 1; 

						/* Hack -- If a rod, staff, or wand, allocate total
						 * maximum timeouts or charges between those
					 	* stolen and those missed. -LM-
					 	*/
						distribute_charges(o_ptr, i_ptr, 1);

						/* Get a description */
						object_desc(Ind, o_name, i_ptr, FALSE, 3);

						/* Message */
						msg_format(Ind, "%sour %s (%c) was stolen!",
						           ((o_ptr->number > 1) ? "One of y" : "Y"),
						           o_name, index_to_label(i));
	
						if	(monster_can_carry(m_idx))
						{
							/* Carry the object */ 
							monster_carry(Ind, m_idx, i_ptr); 
						}
						
						/* Steal the items */
						inven_item_increase(Ind, i, -1);
						inven_item_optimize(Ind, i);

						/* Obvious */
						obvious = TRUE;

						/* Blink away */
						blinked = 2;

						/* Done */
						break;
					}

					break;
				}

				case RBE_EAT_FOOD:
				{
					/* Take some damage */
					take_hit(Ind, damage, ddesc);

					/* Steal some food */
					for (k = 0; k < 10; k++)
					{
						/* Pick an item from the pack */
						i = rand_int(INVEN_PACK);

						/* Get the item */
						o_ptr = &p_ptr->inventory[i];

						/* Accept real items */
						if (!o_ptr->k_idx) continue;

						/* Only eat food */
						if (o_ptr->tval != TV_FOOD) continue;

						/* Get a description */
						object_desc(Ind, o_name, o_ptr, FALSE, 0);

						/* Message */
						msg_format(Ind, "%sour %s (%c) was eaten!",
						           ((o_ptr->number > 1) ? "One of y" : "Y"),
						           o_name, index_to_label(i));

						/* Steal the items */
						inven_item_increase(Ind, i, -1);
						inven_item_optimize(Ind, i);

						/* Obvious */
						obvious = TRUE;

						/* Done */
						break;
					}

					break;
				}

				case RBE_EAT_LITE:
				{
					/* Take some damage */
					take_hit(Ind, damage, ddesc);

					/* Access the lite */
					o_ptr = &p_ptr->inventory[INVEN_LITE];

					/* Drain fuel */
					if ((o_ptr->pval > 0) && (o_ptr->sval < SV_LITE_DWARVEN))
					{
						/* Reduce fuel */
						o_ptr->pval -= (250 + randint(250));
						if (o_ptr->pval < 1) o_ptr->pval = 1;

						/* Notice */
						if (!p_ptr->blind)
						{
							msg_print(Ind, "Your light dims.");
							obvious = TRUE;
						}

						/* Window stuff */
						p_ptr->window |= (PW_INVEN | PW_EQUIP);
					}

					break;
				}

				case RBE_ACID:
				{
					/* Obvious */
					obvious = TRUE;

					/* Message */
					msg_print(Ind, "You are covered in acid!");

					/* Special damage */
					acid_dam(Ind, damage, ddesc);

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_ACID);

					break;
				}

				case RBE_ELEC:
				{
					/* Obvious */
					obvious = TRUE;

					/* Message */
					msg_print(Ind, "You are struck by electricity!");

					/* Special damage */
					elec_dam(Ind, damage, ddesc);

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_ELEC);

					break;
				}

				case RBE_FIRE:
				{
					/* Obvious */
					obvious = TRUE;

					/* Message */
					msg_print(Ind, "You are enveloped in flames!");

					/* Special damage */
					fire_dam(Ind, damage, ddesc);

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_FIRE);

					break;
				}

				case RBE_COLD:
				{
					/* Obvious */
					obvious = TRUE;

					/* Message */
					msg_print(Ind, "You are covered with frost!");

					/* Special damage */
					cold_dam(Ind, damage, ddesc);

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_COLD);

					break;
				}

				case RBE_BLIND:
				{
					/* Take damage */
					take_hit(Ind, damage, ddesc);

					/* Increase "blind" */
					if (!p_ptr->resist_blind)
					{
						if (set_blind(Ind, p_ptr->blind + 10 + randint(rlev)))
						{
							obvious = TRUE;
						}
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_BLIND);

					break;
				}

				case RBE_CONFUSE:
				{
					/* Take damage */
					take_hit(Ind, damage, ddesc);

					/* Increase "confused" */
					if (!p_ptr->resist_conf)
					{
						if (set_confused(Ind, p_ptr->confused + 3 + randint(rlev)))
						{
							obvious = TRUE;
						}
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_CONF);

					break;
				}

				case RBE_TERRIFY:
				{
					/* Take damage */
					take_hit(Ind, damage, ddesc);

					/* Increase "afraid" */
					if (p_ptr->resist_fear)
					{
						msg_print(Ind, "You stand your ground!");
						obvious = TRUE;
					}
					else if (rand_int(100) < p_ptr->skill_sav)
					{
						msg_print(Ind, "You stand your ground!");
						obvious = TRUE;
					}
					else
					{
						if (set_afraid(Ind, p_ptr->afraid + 3 + randint(rlev)))
						{
							obvious = TRUE;
						}
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_FEAR);

					break;
				}

				case RBE_PARALYZE:
				{
					/* Take damage */
					take_hit(Ind, damage, ddesc);

					/* Increase "paralyzed" */
					if (p_ptr->free_act)
					{
						msg_print(Ind, "You are unaffected!");
						obvious = TRUE;
					}
					else if (rand_int(100) < p_ptr->skill_sav)
					{
						msg_print(Ind, "You resist the effects!");
						obvious = TRUE;
					}
					else
					{
						if (set_paralyzed(Ind, p_ptr->paralyzed + 3 + randint(rlev)))
						{
							obvious = TRUE;
							
							/* Hack - Make level 1 monsters who paralyze also blink */
							if (r_ptr->level == 1) blinked = 1;
						}
					}

					/* Learn about the player */
					update_smart_learn(m_idx, DRS_FREE);

					break;
				}

				case RBE_LOSE_STR:
				{
					/* Damage (physical) */
					take_hit(Ind, damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(Ind, A_STR)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_INT:
				{
					/* Damage (physical) */
					take_hit(Ind, damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(Ind, A_INT)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_WIS:
				{
					/* Damage (physical) */
					take_hit(Ind, damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(Ind, A_WIS)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_DEX:
				{
					/* Damage (physical) */
					take_hit(Ind, damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(Ind, A_DEX)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_CON:
				{
					/* Damage (physical) */
					take_hit(Ind, damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(Ind, A_CON)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_CHR:
				{
					/* Damage (physical) */
					take_hit(Ind, damage, ddesc);

					/* Damage (stat) */
					if (do_dec_stat(Ind, A_CHR)) obvious = TRUE;

					break;
				}

				case RBE_LOSE_ALL:
				{
					/* Damage (physical) */
					take_hit(Ind, damage, ddesc);

					/* Damage (stats) */
					if (do_dec_stat(Ind, A_STR)) obvious = TRUE;
					if (do_dec_stat(Ind, A_DEX)) obvious = TRUE;
					if (do_dec_stat(Ind, A_CON)) obvious = TRUE;
					if (do_dec_stat(Ind, A_INT)) obvious = TRUE;
					if (do_dec_stat(Ind, A_WIS)) obvious = TRUE;
					if (do_dec_stat(Ind, A_CHR)) obvious = TRUE;

					break;
				}

				case RBE_SHATTER:
				{
					/* Obvious */
					obvious = TRUE;

					/* Hack -- Reduce damage based on the player armor class */
					damage -= (damage * ((ac < 150) ? ac : 150) / 250);

					/* Take damage */
					take_hit(Ind, damage, ddesc);

					/* Radius 8 earthquake centered at the monster */
					if (damage > 23) earthquake(p_ptr->dun_depth, m_ptr->fy, m_ptr->fx, 8);

					break;
				}

				case RBE_EXP_10:
				{
					/* Obvious */
					obvious = TRUE;

					/* Take damage */
					take_hit(Ind, damage, ddesc);

					if (p_ptr->hold_life && (rand_int(100) < 95))
					{
						msg_print(Ind, "You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(10, 6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
						if (p_ptr->hold_life)
						{
							msg_print(Ind, "You feel your life slipping away!");
							lose_exp(Ind, d/10);
						}
						else
						{
							msg_print(Ind, "You feel your life draining away!");
							lose_exp(Ind, d);
						}
					}
					break;
				}

				case RBE_EXP_20:
				{
					/* Obvious */
					obvious = TRUE;

					/* Take damage */
					take_hit(Ind, damage, ddesc);

					if (p_ptr->hold_life && (rand_int(100) < 90))
					{
						msg_print(Ind, "You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(20, 6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
						if (p_ptr->hold_life)
						{
							msg_print(Ind, "You feel your life slipping away!");
							lose_exp(Ind, d/10);
						}
						else
						{
							msg_print(Ind, "You feel your life draining away!");
							lose_exp(Ind, d);
						}
					}
					break;
				}

				case RBE_EXP_40:
				{
					/* Obvious */
					obvious = TRUE;

					/* Take damage */
					take_hit(Ind, damage, ddesc);

					if (p_ptr->hold_life && (rand_int(100) < 75))
					{
						msg_print(Ind, "You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(40, 6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
						if (p_ptr->hold_life)
						{
							msg_print(Ind, "You feel your life slipping away!");
							lose_exp(Ind, d/10);
						}
						else
						{
							msg_print(Ind, "You feel your life draining away!");
							lose_exp(Ind, d);
						}
					}
					break;
				}

				case RBE_EXP_80:
				{
					/* Obvious */
					obvious = TRUE;

					/* Take damage */
					take_hit(Ind, damage, ddesc);

					if (p_ptr->hold_life && (rand_int(100) < 50))
					{
						msg_print(Ind, "You keep hold of your life force!");
					}
					else
					{
						s32b d = damroll(80, 6) + (p_ptr->exp/100) * MON_DRAIN_LIFE;
						if (p_ptr->hold_life)
						{
							msg_print(Ind, "You feel your life slipping away!");
							lose_exp(Ind, d/10);
						}
						else
						{
							msg_print(Ind, "You feel your life draining away!");
							lose_exp(Ind, d);
						}
					}
					break;
				}
				
				case RBE_HALLU:
				{
					/* Take damage */
					take_hit(Ind, damage, ddesc);

					/* Increase "image" */
					if (!p_ptr->resist_chaos)
					{
						if (set_image(Ind, p_ptr->image + 3 + randint(rlev / 2)))
						{
							obvious = TRUE;
						}
					}
				}
			}


			/* Hack -- only one of cut or stun */
			if (do_cut && do_stun)
			{
				/* Cancel cut */
				if (rand_int(100) < 50)
				{
					do_cut = 0;
				}

				/* Cancel stun */
				else
				{
					do_stun = 0;
				}
			}

			/* Handle cut */
			if (do_cut)
			{
				int k = 0;

				/* Critical hit (zero if non-critical) */
				tmp = monster_critical(d_dice, d_side, damage);

				/* Roll for damage */
				switch (tmp)
				{
					case 0: k = 0; break;
					case 1: k = randint(5); break;
					case 2: k = randint(5) + 5; break;
					case 3: k = randint(20) + 20; break;
					case 4: k = randint(50) + 50; break;
					case 5: k = randint(100) + 100; break;
					case 6: k = 300; break;
					default: k = 500; break;
				}

				/* Apply the cut */
				if (k) (void)set_cut(Ind, p_ptr->cut + k);
			}

			/* Handle stun */
			if (do_stun)
			{
				int k = 0;

				/* Critical hit (zero if non-critical) */
				tmp = monster_critical(d_dice, d_side, damage);

				/* Roll for damage */
				switch (tmp)
				{
					case 0: k = 0; break;
					case 1: k = randint(5); break;
					case 2: k = randint(10) + 10; break;
					case 3: k = randint(20) + 20; break;
					case 4: k = randint(30) + 30; break;
					case 5: k = randint(40) + 40; break;
					case 6: k = 100; break;
					default: k = 200; break;
				}

				/* Apply the stun */
				if (k) (void)set_stun(Ind, p_ptr->stun + k);
			}
		}

		/* Monster missed player */
		else
		{
			/* Analyze failed attacks */
			switch (method)
			{
				case RBM_HIT:
				case RBM_TOUCH:
				case RBM_PUNCH:
				case RBM_KICK:
				case RBM_CLAW:
				case RBM_BITE:
				case RBM_STING:
				case RBM_XXX1:
				case RBM_BUTT:
				case RBM_CRUSH:
				case RBM_ENGULF:
				case RBM_XXX2:

				/* Visible monsters */
				if (p_ptr->mon_vis[m_idx])
				{
					/* Disturbing */
					disturb(Ind, 1, 0);

					/* Message */
					msg_format(Ind, "%^s misses you.", m_name);
					sound(Ind, MSG_MISS);
					slash_fx(Ind, 0 - m_idx, Ind, MSG_MISS);
				}

				break;
			}
		}


		/* Analyze "visible" monsters only */
		if (visible)
		{
			/* Count "obvious" attacks (and ones that cause damage) */
			if (obvious || damage || (l_ptr->blows[ap_cnt] > 10))
			{
				/* Count attacks of this type */
				if (l_ptr->blows[ap_cnt] < MAX_UCHAR)
				{
					l_ptr->blows[ap_cnt]++;
				}
			}
		}
	}


	/* Blink away */
	if (blinked == 2)
	{
		msg_print(Ind, "There is a puff of smoke!");
		teleport_away(m_idx, MAX_SIGHT * 2 + 5);
	}
    else if (blinked == 1)
	{
		msg_format(Ind, "%^s blinks away.", m_name);
		teleport_away(m_idx, 10);
	}


	/* Always notice cause of death */
	if (p_ptr->death && (l_ptr->deaths < MAX_SHORT)) l_ptr->deaths++;


	/* Assume we attacked */
	return (TRUE);
}

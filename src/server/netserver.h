#ifndef __Netserver_h
#define	__Netserver_h


#define NOT_CONNECTED		(-1)

#define CONN_FREE		0x00
#define CONN_SETUP		0x01
#define CONN_XXXX1		0x02
#define CONN_LOGIN		0x04
#define CONN_PLAYING		0x08
#define CONN_DRAIN		0x20
#define CONN_READY		0x40
#define CONN_CONSOLE		0x80

#define SETUP_TIMEOUT		180
#define IDLE_TIMEOUT		15

#define MAX_RTT			(2 * FPS)

#define MIN_RETRANSMIT		(FPS / 8 + 1)
#define MAX_RETRANSMIT		(2 * FPS)
#define DEFAULT_RETRANSMIT	(FPS / 2)

typedef struct {
	int		state;
	int		drain_state;
	sockbuf_t	r;
	sockbuf_t	w;
	sockbuf_t	c;
	sockbuf_t	q;
	hturn		start;
	long		timeout;
	int		client_setup;
	u16b		conntype;
	int		char_state;
	int		id;
	unsigned	version;
	char		*real;
	char 		*nick;
	char		*addr;
	char		*host;
	char		*pass;
	int		race;
	int		class;
	int		sex;
	int		stat_order[6];
	client_setup_t	Client_setup;
	bool		console_authenticated;
	bool		console_listen;
	byte		console_channels[MAX_CHANNELS];
} connection_t;

static void Contact(int fd, int arg);
//static void Console(int fd, int arg);

static int Enter_player(int ind);
void Handle_input(int fd, int arg);
void do_quit(int ind, bool tellclient);

static int Receive_motd(int ind);
static int Receive_char_info(int ind);
static int Receive_verify_visual(int ind);

static int Receive_quit(int ind);
static int Receive_play(int ind);
static int Receive_discard(int ind);
static int Receive_undefined(int ind);

static int Receive_pass(int ind);

static int Receive_keepalive(int ind);
static int Receive_walk(int ind);
static int Receive_run(int ind);
static int Receive_tunnel(int ind);
static int Receive_aim_wand(int ind);
static int Receive_drop(int ind);
static int Receive_fire(int ind);
static int Receive_stand(int ind);
static int Receive_destroy(int ind);
static int Receive_look(int ind);
static int Receive_spell(int ind);
static int Receive_observe(int ind);

static int Receive_open(int ind);
static int Receive_pray(int ind);
static int Receive_ghost(int ind);
static int Receive_quaff(int ind);
static int Receive_read(int ind);
static int Receive_search(int ind);
static int Receive_take_off(int ind);
static int Receive_use(int ind);
static int Receive_throw(int ind);
static int Receive_wield(int ind);
static int Receive_zap(int ind);

static int Receive_target(int ind);
static int Receive_target_friendly(int ind);
static int Receive_inscribe(int ind);
static int Receive_uninscribe(int ind);
static int Receive_activate(int ind);
static int Receive_bash(int ind);
static int Receive_disarm(int ind);
static int Receive_eat(int ind);
static int Receive_fill(int ind);
static int Receive_locate(int ind);
static int Receive_map(int ind);
static int Receive_search_mode(int ind);

static int Receive_close(int ind);
static int Receive_gain(int ind);
static int Receive_direction(int ind);
static int Receive_go_up(int ind);
static int Receive_go_down(int ind);
static int Receive_message(int ind);
static int Receive_channel(int ind);
static int Receive_item(int ind);
static int Receive_purchase(int ind);

static int Receive_sell(int ind);
static int Receive_store_leave(int ind);
static int Receive_store_confirm(int ind);
static int Receive_drop_gold(int ind);
static int Receive_redraw(int ind);
static int Receive_rest(int ind);
static int Receive_party(int ind);
static int Receive_special_line(int ind);
static int Receive_interactive(int ind);
static int Receive_term_key(int ind);
static int Receive_symbol_query(int ind);

static int Receive_spike(int ind);
static int Receive_steal(int ind);
static int Receive_options(int ind);
static int Receive_suicide(int ind);
static int Receive_master(int ind);
static int Receive_clear(int ind);
static int Receive_custom_command(int ind);

static void Handle_item(int Ind, int item);
int Setup_net_server(void);
void Send_item_testers(int ind);
void Send_custom_commands(int ind);
int Send_game_start_conn(int ind);
int Send_motd_conn(int ind, int offset);
int Send_option_info_conn(int ind);
int Send_basic_info_conn(int ind);
int Send_inven_info_conn(int ind);
int Send_race_info_conn(int ind);
int Send_class_info_conn(int ind);
int Send_char_info_conn(int ind);
int Send_term_resize_ack(int ind);

int Setup_net_server(void);
bool Destroy_connection(int ind, char *reason);
int Check_connection(char *real, char *nick, char *addr);
int Setup_connection(char *real, char *nick, char *addr, char *host, char *pass, bool need_info, int race, int class, int sex, u16b conntype, unsigned version, int fd);
int Input(void);
int Send_reply(int ind, int replyto, int result);
int Send_leave(int ind, int id);
int Send_reliable(int ind);

#endif

/* main.c
 *
 * This is the entry point for the system.  System init
 * is performed, then the main app is kicked off.
 */

/* This code initializes the eTPU and a QD instance (simple, no home or index inputs).  The code
   then exercises the function by toggling the input pins.  The input ramps through a very large
   frequency / RPM range in order to show 32-bit period collection. */

/* for sim environment */
#include "isrLib.h"
#include "ScriptLib.h"

/* for eTPU/QD */
#include "etpu_util_ext.h"
#include "etpu_gct.h"
#include "etpu_eqd.h"


#define QD_PHASE_A_CHAN 1
#define QD_PHASE_B_CHAN 2

int32_t g_complete_flag = 0;

uint32_t g_fail_loop_cnt = 0;
void fail_loop()
{
    while (1)
        g_fail_loop_cnt++;
}


/* main application entry point */
/* w/ GNU, if we name this main, it requires linking with the libgcc.a
   run-time support.  This may be useful with C++ because this extra
   code initializes static C++ objects.  However, this C demo will
   skip it */
int user_main()
{
    uint8_t channel_primary = QD_PHASE_A_CHAN;
    uint8_t channel_secondary = QD_PHASE_B_CHAN;
    uint32_t period;
    int8_t direction;
    int24_t pc;
    int24_t pc_sc;
    
	/* initialize interrupt support */
	isrLibInit();
	/* enable interrupt acknowledgement */
	isrEnableAllInterrupts();
	
    /* initialize eTPU */	
    if (my_system_etpu_init())
        return 1;

    /* start eTPU */
    my_system_etpu_start();

    /* note: the below should probably be called in the etpu_gct.c my_system_etpu_init() routine */
    if (fs_etpu_eqd_init(EM_AB, channel_primary, channel_secondary,
        0, 0, FS_ETPU_QD_PRIM_SEC, FS_ETPU_PRIORITY_MIDDLE, FS_ETPU_QD_CONFIGURATION_0,
        FS_ETPU_TCR1, 0, 21000, 19000, 29000, 28000, 
        0x500000, 0xB00000, 
        FS_ETPU_QD_HOME_TRANS_ANY, FS_ETPU_QD_INDEX_PULSE_POSITIVE, FS_ETPU_QD_INDEX_PC_NO_RESET,
        50000000, 60) != 0)
        fail_loop();

    // ********************************************
    // Pin Init States.
    // ********************************************
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );

    // --------------- 1 ------------------
    wait_time(500000);
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    // --------------- 2 ------------------
    wait_time(500000);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(500000);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(500000);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(500000);
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    // --------------- 2 ------------------
    wait_time(400000);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(300000);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(200000);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(100000);
    //verify_chan_data32(QD_PHASE_A_CHAN, FS_ETPU_QD_PERIOD_OFFSET, 50*(500000+400000+300000+200000));
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    period = fs_etpu_eqd_get_period(EM_AB, channel_primary);
    if (period != 50*(500000+400000+300000+200000))
        fail_loop();
    // --------------- 2 ------------------
    wait_time(50000);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(30000);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(20000);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(10000);
    //verify_chan_data32(QD_PHASE_A_CHAN, FS_ETPU_QD_PERIOD_OFFSET, 50*(100000+50000+30000+20000));
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    period = fs_etpu_eqd_get_period(EM_AB, channel_primary);
    if (period != 50*(100000+50000+30000+20000))
        fail_loop();
    // --------------- 2 ------------------
    wait_time(5000);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(3000);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(1500);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(800);
    //verify_chan_data32(QD_PHASE_A_CHAN, FS_ETPU_QD_PERIOD_OFFSET, 50*(10000+5000+3000+1500));
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    period = fs_etpu_eqd_get_period(EM_AB, channel_primary);
    if (period != 50*(10000+5000+3000+1500))
        fail_loop();
    // --------------- 2 ------------------
    wait_time(400);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(200);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(50);
    //verify_chan_data32(QD_PHASE_A_CHAN, FS_ETPU_QD_PERIOD_OFFSET, 50*(800+400+200+100));
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    period = fs_etpu_eqd_get_period(EM_AB, channel_primary);
    if (period != 50*(800+400+200+100))
        fail_loop();
    // --------------- 2 ------------------
    wait_time(50);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(50);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(50);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(50);
    //verify_chan_data32(QD_PHASE_A_CHAN, FS_ETPU_QD_PERIOD_OFFSET, 50*(50+50+50+50));
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    period = fs_etpu_eqd_get_period(EM_AB, channel_primary);
    if (period != 50*(50+50+50+50))
        fail_loop();
    // --------------- 2 ------------------
    wait_time(50);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(50);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(50);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(50);
    //verify_chan_data32(QD_PHASE_A_CHAN, FS_ETPU_QD_PERIOD_OFFSET, 50*(50+50+50+50));
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    period = fs_etpu_eqd_get_period(EM_AB, channel_primary);
    if (period != 50*(50+50+50+50))
        fail_loop();
    // --------------- 2 ------------------
    wait_time(50);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(50);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(50);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(50);
    //verify_chan_data32(QD_PHASE_A_CHAN, FS_ETPU_QD_PERIOD_OFFSET, 50*(50+50+50+50));
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    period = fs_etpu_eqd_get_period(EM_AB, channel_primary);
    if (period != 50*(50+50+50+50))
        fail_loop();
    // --------------- 2 ------------------
    wait_time(50);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(45);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(42);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(40);
    //verify_chan_data32(QD_PHASE_A_CHAN, FS_ETPU_QD_PERIOD_OFFSET, 50*(50+50+45+42));
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    period = fs_etpu_eqd_get_period(EM_AB, channel_primary);
    if (period != 50*(50+50+45+42))
        fail_loop();
    // --------------- 2 ------------------
    wait_time(39);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(38);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(37);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(36);
    //verify_chan_data32(QD_PHASE_A_CHAN, FS_ETPU_QD_PERIOD_OFFSET, 50*(40+39+38+37));
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    period = fs_etpu_eqd_get_period(EM_AB, channel_primary);
    if (period != 50*(40+39+38+37))
        fail_loop();
    // --------------- 2 ------------------
    wait_time(35);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(34);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(33);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(32);
    //verify_chan_data32(QD_PHASE_A_CHAN, FS_ETPU_QD_PERIOD_OFFSET, 50*(36+35+34+33));
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    period = fs_etpu_eqd_get_period(EM_AB, channel_primary);
    if (period != 50*(36+35+34+33))
        fail_loop();
    // --------------- 2 ------------------
    wait_time(31);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(30);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(30);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(30);
    //verify_chan_data32(QD_PHASE_A_CHAN, FS_ETPU_QD_PERIOD_OFFSET, 50*(32+31+30+30));
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    period = fs_etpu_eqd_get_period(EM_AB, channel_primary);
    if (period != 50*(32+31+30+30))
        fail_loop();
    // --------------- 2 ------------------
    wait_time(30);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(30);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(30);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(30);
    //verify_chan_data32(QD_PHASE_A_CHAN, FS_ETPU_QD_PERIOD_OFFSET, 50*(30+30+30+30));
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    // --------------- 2 ------------------
    period = fs_etpu_eqd_get_period(EM_AB, channel_primary);
    if (period != 50*(30+30+30+30))
        fail_loop();
    wait_time(30);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(30);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(30);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(31);
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    // --------------- 2 ------------------
    wait_time(32);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(33);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(34);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(35);
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    // --------------- 2 ------------------
    wait_time(36);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(38);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(40);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(42);
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    // --------------- 2 ------------------
    wait_time(44);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(46);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(48);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(50);
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    // --------------- 2 ------------------
    wait_time(52);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(54);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(56);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(58);
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    // --------------- 2 ------------------
    wait_time(60);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(250);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(500);
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    // --------------- 2 ------------------
    wait_time(1000);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(2000);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(4000);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(8000);
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    // --------------- 2 ------------------
    wait_time(20000);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(50000);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(100000);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 1 ------------------
    wait_time(200000);
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    // --------------- 2 ------------------
    wait_time(300000);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(400000);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(500000);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );

    // verify a few more parameters at this point
    wait_time(10);
    //verify_chan_data8(QD_PHASE_A_CHAN, FS_ETPU_QD_DIRECTION_OFFSET, 1);
    //verify_chan_data24(QD_PHASE_A_CHAN, FS_ETPU_QD_PC_OFFSET, 88);
    //verify_chan_data24(QD_PHASE_A_CHAN, FS_ETPU_QD_PC_SC_OFFSET, 88);
    direction = fs_etpu_eqd_get_direction(EM_AB, channel_primary);
    if (direction != FS_ETPU_QD_DIRECTION_INC)
        fail_loop();
    pc = fs_etpu_eqd_get_pc(EM_AB, channel_primary);
    if (pc != 88)
        fail_loop();
    pc_sc = fs_etpu_eqd_get_pc_sc(EM_AB, channel_primary);
    if (pc_sc != 88)
        fail_loop();


    // --------------- 1 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    // --------------- 2 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 3 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 4 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 5 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    // --------------- 6 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 7 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 8 ------------------
    wait_time(100);
    // --------------- 9 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    // --------------- 10 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 11 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 12 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 13 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    // --------------- 14 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    direction = fs_etpu_eqd_get_direction(EM_AB, channel_primary);
    if (direction != FS_ETPU_QD_DIRECTION_DEC)
        fail_loop();
    // --------------- 15 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 16 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 17 ------------------
    wait_time(100);
    // --------------- 18 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );
    // --------------- 19 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_A_CHAN, 1 );
    // --------------- 20 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_B_CHAN, 1 );
    // --------------- 21 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_A_CHAN, 0 );
    // --------------- 22 ------------------
    wait_time(100);
    write_chan_input_pin(QD_PHASE_B_CHAN, 0 );

    wait_time(10);
    //verify_chan_data8(QD_PHASE_A_CHAN, FS_ETPU_QD_DIRECTION_OFFSET, 1);
    //verify_chan_data24(QD_PHASE_A_CHAN, FS_ETPU_QD_PC_OFFSET, 4);
    //verify_chan_data24(QD_PHASE_A_CHAN, FS_ETPU_QD_PC_SC_OFFSET, 4);
    direction = fs_etpu_eqd_get_direction(EM_AB, channel_primary);
    if (direction != FS_ETPU_QD_DIRECTION_INC)
        fail_loop();
    pc = fs_etpu_eqd_get_pc(EM_AB, channel_primary);
    if (pc != 92)
        fail_loop();
    pc_sc = fs_etpu_eqd_get_pc_sc(EM_AB, channel_primary);
    if (pc_sc != 92)
        fail_loop();



	/* TESTING DONE */
	
	g_complete_flag = 1;

	while (1)
		;

	return 0;
}

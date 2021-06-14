/*
 * app.h
 *
 * Created: 05.06.2021 17:31:24
 *  Author: alrha
 */



#if defined(NDEBUG) || defined(DEBUGCFG)
	#ifndef APPCFG_H_
	#define APPCFG_H_



	namespace apps{
		class appcfg{
			public:
			appcfg( const appcfg& ) = delete; // non construction-copyable
			appcfg& operator=( const appcfg& ) = delete; // non copyable
			appcfg();
			~appcfg();

			bool on_entry();
			bool on_run();
			bool on_exit();
		};
	}


	#endif /* APPCFG_H_ */
#endif
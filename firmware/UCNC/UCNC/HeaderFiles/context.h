/*
 * context.h
 *
 * Created: 05.06.2021 17:26:08
 *  Author: alrha
 */


#ifndef CONTEXT_H_
#define CONTEXT_H_
namespace core{


	template<typename app_type>
	class app_context : public app_type{
		public:
		app_context<app_type>( const app_context<app_type>& ) = delete; // non construction-copyable
		app_context<app_type>& operator=( const app_context<app_type>& ) = delete; // non copyable
		app_context<app_type>(){}
		private:

		bool entry(){
			return this->on_entry();
		}

		bool run(){
			return this->on_run();
		}

		bool exit(){
			return this->on_exit();
		}
		public:

		static void execute(){
			app_context<app_type> app;
			if(app.entry()){
				while(app.run());
			}
			app.exit();
		}
	};


};

#endif /* CONTEXT_H_ */
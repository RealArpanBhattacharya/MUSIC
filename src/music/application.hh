#ifndef MUSIC_APPLICATION_HH
#include "music/music-config.hh"

#if MUSIC_USE_MPI
#include <map>
#include <string>
#include <iostream>
#include <sstream>
#include <memory>
#include <mpi.h>
#include "music/configuration.hh"
#include "music/error.hh"
#include "music/port.hh"
#include "music/runtime.hh"
#include "music/connectivity.hh"
#include "music/music_context.hh"
#include "music/port_manager.hh"
#include "music/misc.hh"


namespace MUSIC
{

	enum class ApplicationState {RUNNING, STOPPED, FINALIZED};
	const double MUSIC_DEFAULT_TIMEBASE = 1e-9;
	class PortConnectivityManager;

	class Application final
	{
		private:
			Application(std::unique_ptr<Configuration> config, MPI_Comm comm, bool launchedByMusic, double timebase);

		public:
			Application ();
			Application (std::unique_ptr<MusicContext> context,
					double timebase = MUSIC_DEFAULT_TIMEBASE);

			/* Application(Configuration config, double h, MPI::MPI_Comm comm); */

			Application(const Application&) = delete;
			Application& operator= (const Application&) = delete;

			// TODO move and move-assignment constructors

			void tick();
			void enterSimulationLoop (double h);
			void exitSimulationLoop ();
			void finalize();

			double time();
			double timebase() const;
			std::string name () const;
			bool launchedByMusic () const;

			template <class PortT>
			std::shared_ptr<PortT> publish (std::string identifier)
			{
				return port_manager_.createPort<PortT> (*this, identifier);
			}

			PortConnectivityManager& getPortConnectivityManager () const;

			/* std::shared_ptr<ContInputPort> publishContInput (std::string identifier); */
			/* std::shared_ptr<ContOutputPort> publishContOutput (std::string identifier); */
			/* std::shared_ptr<EventInputPort> publishEventInput (std::string identifier); */
			/* std::shared_ptr<EventOutputPort> publishEventOutput (std::string identifier); */
			/* std::shared_ptr<MessageInputPort> publishMessageInput (std::string identifier); */
			/* std::shared_ptr<MessageOutputPort> publishMessageOutput (std::string identifier); */

		private:
			std::unique_ptr<Configuration> config_;
			double timebase_;
			MPI::Intracomm comm_ {MPI::COMM_WORLD};
			ApplicationState state_ {ApplicationState::STOPPED};
			bool launchedByMusic_ {false};
			/* std::unique_ptr<ApplicationMap> application_map_; */
			PortConnectivityManager port_manager_;


		private:
			friend class Port;
			friend class TemporalNegotiator;
			friend class Runtime;

			std::unique_ptr<Runtime> runtime_;
			void assertValidState(std::string func_name, ApplicationState as);

			const ApplicationMap& applicationMap () const;
			MPI::Intracomm communicator () const;
			int applicationColor() const;
			int nProcs () const;
			int leader () const;


	};

	inline double Application::time()
	{
		return runtime_->time();
	}

	inline void Application::tick()
	{
		assertValidState("tick", ApplicationState::RUNNING);
		runtime_->tick();
	}


	inline void Application::assertValidState(std::string func_name, ApplicationState as)
	{
		if (as == state_)
			return;
		std::stringstream ss;
		ss << "Calling " << func_name << " is permitted in "<< state_;
		std::string s = ss.str();
		errorRank(s);
	}

}
#endif
#define MUSIC_APPLICATION_HH
#endif

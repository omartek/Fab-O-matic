#ifndef MACHINE_H_
#define MACHINE_H_

#include <cstdint>
#include <array>
#include <chrono>
#include "FabUser.hpp"
#include "MachineConfig.hpp"

namespace fablabbg
{
  class FabServer;

  class Machine
  {
  public:
    enum class PowerState
    {
      POWERED_ON,
      WAITING_FOR_POWER_OFF,
      POWERED_OFF,
      UNKNOWN
    };

    Machine();
    ~Machine() = default;

    bool maintenanceNeeded; // If true, machine needs maintenance
    bool allowed;           // If false, nobody can use the machine

    FabUser &getActiveUser();
    void configure(MachineConfig new_config, FabServer &serv); // Must be called before using the machine
    MachineID getMachineId() const;
    std::string getMachineName() const;
    seconds getUsageDuration() const;
    minutes getAutologoffDelay() const;

    bool login(FabUser user);                   // if the machine is not active, login the user
    void logout();                              // if the machine is active, check if the card belongs to the user that is logged in and logout the user
    void setAutologoffDelay(minutes new_delay); // Sets the delay after which the user will be logged off automatically
    void power(bool on_or_off);                 // Power-on or off the machine

    PowerState getPowerState() const; // Gets the current state of the machine
    bool isShutdownImminent() const;  // True if the machine will power down in less than BEEP_REMAINING_MINUTES
    bool isFree() const;              // True is the machine is not used by anybody
    bool canPowerOff() const;         // True if POWEROFF_DELAY_MINUTES delay has expired,and the machine is still idle
    std::string toString() const;
    bool isAutologoffExpired() const;

  private:
    std::optional<MachineConfig> config;
    std::optional<std::reference_wrapper<FabServer>> server;

    bool active;
    FabUser current_user;

    std::optional<time_point<system_clock>> usage_start_timestamp; // When did the machine start?
    std::optional<time_point<system_clock>> logoff_timestamp;      // When did the last user log off?
    PowerState power_state;

    void power_mqtt(bool on_or_off);
    void power_relay(bool on_or_off); // Power-on or off the machine
  };
} // namespace fablabbg
#endif // MACHINE_H_
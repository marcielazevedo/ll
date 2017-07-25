/**
 * The Forgotten Server - a free and open-source MMORPG server emulator
 * Copyright (C) 2014  Mark Samman <mark.samman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef FS_ProtocolCaster_H
#define FS_ProtocolCaster_H

#include "protocolgame.h"

class ProtocolCaster : public ProtocolGame
{
	public:
		ProtocolCaster(Connection_ptr connection);

		typedef std::unordered_map<Player*, ProtocolCaster*> LiveCastsMap;
		typedef std::vector<ProtocolGame*> CastSpectatorVec;

		/** \brief Adds a spectator from the spectators vector.
		 *  \param spectatorClient pointer to the \ref ProtocolSpectator object representing the spectator
		 */
		void addSpectator(ProtocolGame* spectatorClient);

		/** \brief Removes a spectator from the spectators vector.
		 *  \param spectatorClient pointer to the \ref ProtocolSpectator object representing the spectator
		 */
		void removeSpectator(ProtocolGame* spectatorClient);

		/** \brief Starts the live cast.
		 *  \param password live cast password(optional)
		 *  \returns bool type indicating whether starting the cast was successful
		*/
		bool startLiveCast(const std::string& password = "");

		/** \brief Stops the live cast and disconnects all spectators.
		 *  \returns bool type indicating whether stopping the cast was successful
		*/
		bool stopLiveCast();

		/** \brief Provides access to the spectator vector.
		 *  \returns const reference to the spectator vector
		 */
		const CastSpectatorVec& getLiveCastSpectators() const {
			return m_spectators;
		}

		/** \brief Provides information about spectator count.
		 */
		size_t getSpectatorCount() const {
			return m_spectators.size();
		}

		bool isLiveCaster() const {
			return m_isLiveCaster;
		}

		/** \brief Adds a new live cast to the list of available casts
		 *  \param player pointer to the casting \ref Player object
		 *  \param client pointer to the caster's \ref ProtocolGame object
		 */
		void registerLiveCast();

		/** \brief Removes a live cast from the list of available casts
		 *  \param player pointer to the casting \ref Player object
		 */
		void unregisterLiveCast();

		/** \brief Update live cast info in the database.
		 *  \param player pointer to the casting \ref Player object
		 *  \param client pointer to the caster's \ref ProtocolGame object
		 */
		void updateLiveCastInfo();

		/** \brief Clears all live casts. Used to make sure there aro no live cast db rows left should a crash occur.
		 *  \warning Only supposed to be called once.
		 */
		static void clearLiveCastInfo();

		/** \brief Finds the caster's \ref ProtocolGame object
		 *  \param player pointer to the casting \ref Player object
		 *  \returns A pointer to the \ref ProtocolGame of the caster
		 */
		static ProtocolCaster* getLiveCast(Player* player) {
			const auto it = m_liveCasts.find(player);
			return it != m_liveCasts.end() ? it->second : nullptr;
		}

		/** \brief Gets the live cast name/login
		 *  \returns A const reference to a string containing the live cast name/login
		 */
		const std::string& getLiveCastName() const {
			return m_liveCastName;
		}
		/** \brief Gets the live cast password
		 *  \returns A const reference to a string containing the live cast password
		 */
		const std::string& getLiveCastPassword() const {
			return m_liveCastPassword;
		}
		/** \brief Check if the live cast is password protected
		 */
		bool isPasswordProtected() const {
			return !m_liveCastPassword.empty();
		}
		/** \brief Allows access to the live cast map.
		 *  \returns A const reference to the live cast map.
		 */
		static const LiveCastsMap& getLiveCasts() {
			return m_liveCasts;
		}

		static uint8_t getMaxLiveCastCount() {
			return 200;
		}

		ProtocolGame* getSpectatorByName(std::string name);

		/** \brief Allows spectators to send text messages to the caster
		*   and then get broadcast to the rest of the spectators
		*  \param text string containing the text message
		*/
		void broadcastSpectatorMessage(const std::string& name, const std::string& text) {
			if (getConnection() && player) {
				sendChannelMessage(name, text, TALKTYPE_CHANNEL_Y, CHANNEL_CAST);
			}
		}

	protected:
		//proxy functions

		void writeToOutputBuffer(const NetworkMessage& msg, bool broadcast = true) override {
			ProtocolGame::writeToOutputBuffer(msg);

			if (!m_isLiveCaster)
				return;

			if (!broadcast)
				return;

			for (auto& spectator : m_spectators) {
				spectator->writeToOutputBuffer(msg, broadcast);
			}
		}

	private:

		void releaseProtocol() override;

		void disconnectClient(const std::string& message) override;

		void parsePacket(NetworkMessage& msg) override;

		static LiveCastsMap m_liveCasts; ///< Stores all available casts.

		bool m_isLiveCaster; ///< Determines if this \ref ProtocolGame object is casting

		///< list of spectators \warning This variable should only be accessed after locking \ref liveCastLock
		CastSpectatorVec m_spectators;

		// just to name spectators with a number
		uint32_t m_spectatorsCount;

		///< Live cast name that is also used as login
		std::string m_liveCastName;
		///< Password used to access the live cast
		std::string m_liveCastPassword;
};

#endif

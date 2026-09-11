let pollingInterval = null;

function updateBotUI(botId, data) {
  const card = document.getElementById(`bot${botId}-card`);
  const status = document.getElementById(`bot${botId}-status`);
  const distEl = document.getElementById(`bot${botId}-distance`);
  const blockedEl = document.getElementById(`bot${botId}-blocked`);
  const peerEl = document.getElementById(`bot${botId}-peer`);

  if (!data) {
    status.textContent = 'OFFLINE';
    status.className = 'status-indicator';
    distEl.textContent = '-- cm';
    distEl.className = 'value';
    blockedEl.textContent = '--';
    blockedEl.className = 'value';
    peerEl.textContent = '--';
    peerEl.className = 'value';
    card.classList.remove('alert');
    return;
  }

  status.textContent = 'ONLINE';
  status.className = 'status-indicator online';

  distEl.textContent = `${data.local_distance.toFixed(1)} cm`;
  
  if (data.local_blocked) {
    blockedEl.textContent = 'DETECTED';
    blockedEl.className = 'value danger';
    card.classList.add('alert');
  } else {
    blockedEl.textContent = 'CLEAR';
    blockedEl.className = 'value safe';
    card.classList.remove('alert');
  }

  if (data.peer_blocked) {
    peerEl.textContent = 'PEER BLOCKED';
    peerEl.className = 'value danger';
  } else {
    peerEl.textContent = 'SYNCED';
    peerEl.className = 'value safe';
  }
}

async function fetchBotData(ip, botId) {
  try {
    const response = await fetch(`http://${ip}/data`, { 
      method: 'GET',
      signal: AbortSignal.timeout(1000) 
    });
    if (!response.ok) throw new Error('Network response was not ok');
    const data = await response.json();
    updateBotUI(botId, data);
  } catch (error) {
    updateBotUI(botId, null);
  }
}

document.getElementById('connect-btn').addEventListener('click', () => {
  const btn = document.getElementById('connect-btn');
  const ip1 = document.getElementById('bot1-ip').value;
  const ip2 = document.getElementById('bot2-ip').value;

  if (pollingInterval) {
    clearInterval(pollingInterval);
    pollingInterval = null;
    btn.textContent = 'INITIATE RADAR SCAN';
    btn.style.background = 'rgba(0, 255, 204, 0.1)';
    btn.style.color = 'var(--accent)';
    updateBotUI(1, null);
    updateBotUI(2, null);
  } else {
    btn.textContent = 'TERMINATE SCAN';
    btn.style.background = 'var(--alert)';
    btn.style.color = '#fff';
    btn.style.borderColor = 'var(--alert)';
    
    // Poll every 500ms
    pollingInterval = setInterval(() => {
      fetchBotData(ip1, 1);
      fetchBotData(ip2, 2);
    }, 500);
  }
});

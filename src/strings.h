#pragma once
// Player-visible notices in every language Enshrouded supports. This file is UTF-8; build with
// /utf-8 so the literals stay UTF-8. The language follows the game's own setting (see
// popup::Language), or [Messages] Language in map-teleport.ini.
#include <cstdint>
#include <cstring>
#include <string>
#include <string_view>

namespace teleport::text {

enum class Lang : uint8_t { en, fr, it, de, es, ja, ko, pl, pt, ru, zhHans, zhHant, tr, uk, th, count };
inline constexpr const char* kCodes[]={"en","fr","it","de","es","ja","ko","pl","pt","ru","zh-hans","zh-hant","tr","uk","th"};

enum Id {
    Title, InProgress, HostBusy, StillChecking, ServerMissing, NoAccount, SlowPreparing, NoArrival,
    Preparing, Countdown /* one %u: seconds left */, ThankYou, PrankThankYou, Failed,
    Scenario0Opening, Scenario0Problem, Scenario0Correction,
    Scenario1Opening, Scenario1Problem, Scenario1Correction,
    Scenario2Opening, Scenario2Problem, Scenario2Correction,
    IdCount
};

inline constexpr const char* kTexts[size_t(Lang::count)][IdCount]={
// en
{"Embervale Teleport Services",
 "A teleport is already in progress.",
 "The host is busy or teleporting is unavailable. Please try again shortly.",
 "Still checking whether this server runs Embervale Teleport. Please try again in a few seconds.",
 "This server does not run Embervale Teleport, so map teleport is unavailable here. The server needs the mod installed too.",
 "Could not identify this player's account. Teleport cancelled.",
 "Teleport preparation is taking longer than expected. Your request is still pending.",
 "The host has not confirmed arrival. Please wait before trying another teleport.",
 "Preparing your teleport...",
 "Teleporting in %u...",
 "Thank you for choosing Embervale Teleport Services.",
 "We assure you this service error will not happen again. Thank you for choosing Embervale Teleport Services.",
 "Teleport could not be completed. Check the host's Map Teleport log.",
 "You have been upgraded to the deluxe scenic teleport experience. For your safety, we have disabled your glider. Enjoy the ride, and thank you for choosing Embervale Teleport Services.",
 "There has been a small mistake. Please hold on one second.",
 "We apologize. Flameborn appear to react poorly to gravity. This accidental deluxe upgrade will not happen again. All future trips use your selected service. Returning you safely to the ground now.",
 "Your destination was entered in metres. Our system processed it in kilometres. Please enjoy this complimentary aerial preview of your destination.",
 "Our measurements department has requested one moment of your patience.",
 "We have located the decimal point. It was on holiday. We have revoked its leave, so this error will not happen again. Correcting your destination now. Thank you for choosing Embervale Teleport Services.",
 "Your teleport is complete. Please remain still while we locate the ground. It was here when we booked this.",
 "We have located the ground. Unfortunately, it is approaching ahead of schedule.",
 "Please disregard the scenery moving rapidly past you. We are cancelling your appointment with the ground and arranging a gentler introduction."},
// fr
{"Services de téléportation d'Embervale",
 "Une téléportation est déjà en cours.",
 "L'hôte est occupé ou la téléportation est indisponible. Veuillez réessayer dans un instant.",
 "Vérification en cours : ce serveur utilise-t-il Embervale Teleport ? Veuillez réessayer dans quelques secondes.",
 "Ce serveur n'utilise pas Embervale Teleport : la téléportation par la carte n'est pas disponible ici. Le mod doit aussi être installé sur le serveur.",
 "Impossible d'identifier le compte de ce joueur. Téléportation annulée.",
 "La préparation de la téléportation prend plus de temps que prévu. Votre demande est toujours en attente.",
 "L'hôte n'a pas confirmé votre arrivée. Veuillez patienter avant de tenter une autre téléportation.",
 "Préparation de votre téléportation...",
 "Téléportation dans %u...",
 "Merci d'avoir choisi les Services de téléportation d'Embervale.",
 "Nous vous assurons que cette erreur de service ne se reproduira pas. Merci d'avoir choisi les Services de téléportation d'Embervale.",
 "La téléportation n'a pas pu aboutir. Consultez le journal Map Teleport de l'hôte.",
 "Vous avez été surclassé vers l'expérience de téléportation panoramique de luxe. Pour votre sécurité, nous avons désactivé votre planeur. Bon voyage, et merci d'avoir choisi les Services de téléportation d'Embervale.",
 "Une petite erreur s'est produite. Veuillez patienter une seconde.",
 "Toutes nos excuses. Les Nés de la Flamme semblent mal réagir à la gravité. Ce surclassement de luxe accidentel ne se reproduira pas. Tous vos prochains trajets utiliseront le service choisi. Nous vous ramenons maintenant au sol en toute sécurité.",
 "Votre destination a été saisie en mètres. Notre système l'a traitée en kilomètres. Profitez de cet aperçu aérien gratuit de votre destination.",
 "Notre service des mesures vous demande un instant de patience.",
 "Nous avons retrouvé la virgule. Elle était en vacances. Nous avons annulé ses congés, cette erreur ne se reproduira donc pas. Correction de votre destination en cours. Merci d'avoir choisi les Services de téléportation d'Embervale.",
 "Votre téléportation est terminée. Veuillez rester immobile pendant que nous localisons le sol. Il était là quand nous avons fait la réservation.",
 "Nous avons localisé le sol. Malheureusement, il arrive en avance sur l'horaire.",
 "Veuillez ignorer le paysage qui défile rapidement autour de vous. Nous annulons votre rendez-vous avec le sol et organisons une rencontre plus douce."},
// it
{"Servizi di Teletrasporto di Embervale",
 "Un teletrasporto è già in corso.",
 "L'host è occupato o il teletrasporto non è disponibile. Riprova tra poco.",
 "Stiamo ancora verificando se questo server usa Embervale Teleport. Riprova tra qualche secondo.",
 "Questo server non usa Embervale Teleport, quindi qui il teletrasporto dalla mappa non è disponibile. La mod deve essere installata anche sul server.",
 "Impossibile identificare l'account di questo giocatore. Teletrasporto annullato.",
 "La preparazione del teletrasporto richiede più tempo del previsto. La tua richiesta è ancora in attesa.",
 "L'host non ha confermato il tuo arrivo. Attendi prima di tentare un altro teletrasporto.",
 "Preparazione del teletrasporto...",
 "Teletrasporto tra %u...",
 "Grazie per aver scelto i Servizi di Teletrasporto di Embervale.",
 "Ti assicuriamo che questo disservizio non si ripeterà. Grazie per aver scelto i Servizi di Teletrasporto di Embervale.",
 "Impossibile completare il teletrasporto. Controlla il log di Map Teleport dell'host.",
 "Sei stato promosso all'esperienza di teletrasporto panoramico deluxe. Per la tua sicurezza, abbiamo disattivato il tuo aliante. Goditi il viaggio e grazie per aver scelto i Servizi di Teletrasporto di Embervale.",
 "C'è stato un piccolo errore. Attendi un secondo, per favore.",
 "Ci scusiamo. I Nati dalla Fiamma sembrano reagire male alla gravità. Questa promozione deluxe accidentale non si ripeterà. Tutti i prossimi viaggi useranno il servizio che hai scelto. Ti stiamo riportando a terra in sicurezza.",
 "La tua destinazione è stata inserita in metri. Il nostro sistema l'ha elaborata in chilometri. Goditi questa anteprima aerea gratuita della tua destinazione.",
 "Il nostro reparto misure ti chiede un attimo di pazienza.",
 "Abbiamo trovato la virgola. Era in vacanza. Abbiamo revocato le sue ferie, quindi questo errore non si ripeterà. Stiamo correggendo la tua destinazione. Grazie per aver scelto i Servizi di Teletrasporto di Embervale.",
 "Il teletrasporto è completato. Resta fermo mentre cerchiamo il terreno. Era qui quando abbiamo prenotato.",
 "Abbiamo trovato il terreno. Purtroppo si sta avvicinando in anticipo sulla tabella di marcia.",
 "Ignora il paesaggio che ti scorre accanto a gran velocità. Stiamo annullando il tuo appuntamento con il terreno e organizzando un incontro più delicato."},
// de
{"Embervale Teleport-Dienste",
 "Ein Teleport läuft bereits.",
 "Der Host ist beschäftigt oder Teleportieren ist nicht verfügbar. Bitte versuche es gleich noch einmal.",
 "Es wird noch geprüft, ob dieser Server Embervale Teleport verwendet. Bitte versuche es in ein paar Sekunden erneut.",
 "Dieser Server verwendet Embervale Teleport nicht, daher ist der Kartenteleport hier nicht verfügbar. Die Mod muss auch auf dem Server installiert sein.",
 "Das Konto dieses Spielers konnte nicht ermittelt werden. Teleport abgebrochen.",
 "Die Vorbereitung des Teleports dauert länger als erwartet. Deine Anfrage ist noch offen.",
 "Der Host hat deine Ankunft nicht bestätigt. Bitte warte, bevor du erneut teleportierst.",
 "Dein Teleport wird vorbereitet...",
 "Teleport in %u...",
 "Danke, dass du dich für die Embervale Teleport-Dienste entschieden hast.",
 "Wir versichern dir, dass dieser Servicefehler nicht wieder vorkommt. Danke, dass du dich für die Embervale Teleport-Dienste entschieden hast.",
 "Der Teleport konnte nicht abgeschlossen werden. Sieh im Map-Teleport-Log des Hosts nach.",
 "Du wurdest auf das Deluxe-Panorama-Teleporterlebnis hochgestuft. Zu deiner Sicherheit haben wir deinen Gleiter deaktiviert. Genieß die Fahrt, und danke, dass du dich für die Embervale Teleport-Dienste entschieden hast.",
 "Es ist ein kleiner Fehler passiert. Bitte einen Moment Geduld.",
 "Wir entschuldigen uns. Flammengeborene scheinen schlecht auf Schwerkraft zu reagieren. Dieses versehentliche Deluxe-Upgrade wird nicht wieder vorkommen. Alle künftigen Reisen nutzen den von dir gewählten Service. Wir bringen dich jetzt sicher zurück auf den Boden.",
 "Dein Ziel wurde in Metern eingegeben. Unser System hat es in Kilometern verarbeitet. Genieß diese kostenlose Luftvorschau deines Ziels.",
 "Unsere Abteilung für Maßeinheiten bittet dich um einen Moment Geduld.",
 "Wir haben das Komma gefunden. Es war im Urlaub. Wir haben seinen Urlaub gestrichen, dieser Fehler wird also nicht wieder vorkommen. Dein Ziel wird jetzt korrigiert. Danke, dass du dich für die Embervale Teleport-Dienste entschieden hast.",
 "Dein Teleport ist abgeschlossen. Bitte bleib ruhig stehen, während wir den Boden suchen. Er war noch da, als wir gebucht haben.",
 "Wir haben den Boden gefunden. Leider nähert er sich früher als geplant.",
 "Bitte ignoriere die Landschaft, die schnell an dir vorbeizieht. Wir sagen deinen Termin mit dem Boden ab und arrangieren eine sanftere Begegnung."},
// es
{"Servicios de Teletransporte de Embervale",
 "Ya hay un teletransporte en curso.",
 "El anfitrión está ocupado o el teletransporte no está disponible. Inténtalo de nuevo en breve.",
 "Todavía estamos comprobando si este servidor usa Embervale Teleport. Inténtalo de nuevo en unos segundos.",
 "Este servidor no usa Embervale Teleport, así que aquí no se puede teletransportar desde el mapa. El mod también debe estar instalado en el servidor.",
 "No se pudo identificar la cuenta de este jugador. Teletransporte cancelado.",
 "La preparación del teletransporte está tardando más de lo previsto. Tu solicitud sigue pendiente.",
 "El anfitrión no ha confirmado tu llegada. Espera antes de intentar otro teletransporte.",
 "Preparando tu teletransporte...",
 "Teletransporte en %u...",
 "Gracias por elegir los Servicios de Teletransporte de Embervale.",
 "Te aseguramos que este error del servicio no volverá a ocurrir. Gracias por elegir los Servicios de Teletransporte de Embervale.",
 "No se pudo completar el teletransporte. Revisa el registro de Map Teleport del anfitrión.",
 "Te hemos mejorado a la experiencia de teletransporte panorámico de lujo. Por tu seguridad, hemos desactivado tu planeador. Disfruta del viaje, y gracias por elegir los Servicios de Teletransporte de Embervale.",
 "Se ha producido un pequeño error. Espera un segundo, por favor.",
 "Te pedimos disculpas. Parece que los Nacidos de la Llama reaccionan mal a la gravedad. Esta mejora de lujo accidental no volverá a ocurrir. Todos tus próximos viajes usarán el servicio que elegiste. Te estamos devolviendo al suelo de forma segura.",
 "Tu destino se introdujo en metros. Nuestro sistema lo procesó en kilómetros. Disfruta de esta vista aérea gratuita de tu destino.",
 "Nuestro departamento de medidas te pide un momento de paciencia.",
 "Hemos encontrado la coma decimal. Estaba de vacaciones. Le hemos cancelado el permiso, así que este error no volverá a ocurrir. Corrigiendo tu destino. Gracias por elegir los Servicios de Teletransporte de Embervale.",
 "Tu teletransporte se ha completado. Quédate quieto mientras localizamos el suelo. Estaba aquí cuando hicimos la reserva.",
 "Hemos localizado el suelo. Por desgracia, se está acercando antes de lo previsto.",
 "Ignora el paisaje que pasa rápidamente a tu lado. Estamos cancelando tu cita con el suelo y organizando un encuentro más suave."},
// ja
{"Embervale テレポートサービス",
 "テレポートはすでに進行中です。",
 "ホストが混み合っているか、テレポートを利用できません。しばらくしてからもう一度お試しください。",
 "このサーバーで Embervale Teleport が使われているか確認中です。数秒後にもう一度お試しください。",
 "このサーバーでは Embervale Teleport が使われていないため、マップからのテレポートは利用できません。サーバーにも MOD をインストールする必要があります。",
 "このプレイヤーのアカウントを特定できませんでした。テレポートを中止しました。",
 "テレポートの準備に予想より時間がかかっています。リクエストはまだ処理待ちです。",
 "ホストが到着を確認していません。次のテレポートまでしばらくお待ちください。",
 "テレポートの準備中...",
 "テレポートまで %u...",
 "Embervale テレポートサービスをご利用いただき、ありがとうございます。",
 "このようなサービスエラーが二度と起こらないことをお約束します。Embervale テレポートサービスをご利用いただき、ありがとうございます。",
 "テレポートを完了できませんでした。ホストの Map Teleport ログを確認してください。",
 "お客様はデラックス絶景テレポート体験にアップグレードされました。安全のため、グライダーは無効にしてあります。空の旅をお楽しみください。Embervale テレポートサービスをご利用いただき、ありがとうございます。",
 "ちょっとした手違いがありました。少々お待ちください。",
 "大変申し訳ございません。フレイムボーンは重力との相性があまり良くないようです。この誤ったデラックスアップグレードが再び起こることはありません。今後の旅はすべてお選びのサービスで行われます。ただいま安全に地上へお戻しします。",
 "目的地はメートルで入力されましたが、当システムはキロメートルとして処理しました。目的地の無料空中プレビューをお楽しみください。",
 "計測部門より、少々お時間をいただきたいとのことです。",
 "小数点が見つかりました。休暇中でした。休暇は取り消しましたので、このエラーが再び起こることはありません。ただいま目的地を修正しています。Embervale テレポートサービスをご利用いただき、ありがとうございます。",
 "テレポートが完了しました。地面を探していますので、そのままお待ちください。予約したときはここにあったのですが。",
 "地面が見つかりました。残念ながら、予定より早く近づいてきています。",
 "高速で通り過ぎていく景色はお気になさらないでください。地面とのご予約をキャンセルし、もっと穏やかな対面をご用意いたします。"},
// ko
{"Embervale 텔레포트 서비스",
 "이미 텔레포트가 진행 중입니다.",
 "호스트가 바쁘거나 텔레포트를 사용할 수 없습니다. 잠시 후 다시 시도해 주세요.",
 "이 서버에서 Embervale Teleport를 사용하는지 아직 확인 중입니다. 몇 초 후 다시 시도해 주세요.",
 "이 서버는 Embervale Teleport를 사용하지 않아 지도 텔레포트를 사용할 수 없습니다. 서버에도 모드를 설치해야 합니다.",
 "이 플레이어의 계정을 확인할 수 없습니다. 텔레포트가 취소되었습니다.",
 "텔레포트 준비가 예상보다 오래 걸리고 있습니다. 요청은 아직 대기 중입니다.",
 "호스트가 도착을 확인하지 않았습니다. 다른 텔레포트를 시도하기 전에 잠시 기다려 주세요.",
 "텔레포트 준비 중...",
 "%u초 후 텔레포트...",
 "Embervale 텔레포트 서비스를 이용해 주셔서 감사합니다.",
 "이런 서비스 오류는 다시는 일어나지 않을 것을 약속드립니다. Embervale 텔레포트 서비스를 이용해 주셔서 감사합니다.",
 "텔레포트를 완료하지 못했습니다. 호스트의 Map Teleport 로그를 확인하세요.",
 "고객님은 디럭스 경치 감상 텔레포트로 업그레이드되었습니다. 안전을 위해 글라이더를 비활성화했습니다. 즐거운 여행 되시고, Embervale 텔레포트 서비스를 이용해 주셔서 감사합니다.",
 "작은 실수가 있었습니다. 잠시만 기다려 주세요.",
 "죄송합니다. 불꽃의 후예는 중력에 잘 적응하지 못하는 것 같습니다. 이번 실수로 인한 디럭스 업그레이드는 다시 일어나지 않습니다. 앞으로의 모든 여행은 선택하신 서비스로 진행됩니다. 지금 안전하게 지상으로 모셔다 드리겠습니다.",
 "목적지가 미터로 입력되었는데 저희 시스템이 킬로미터로 처리했습니다. 목적지의 무료 항공 미리보기를 즐겨 주세요.",
 "측정 부서에서 잠시만 기다려 달라고 요청드립니다.",
 "소수점을 찾았습니다. 휴가 중이었습니다. 휴가를 취소했으니 이 오류는 다시 일어나지 않습니다. 지금 목적지를 수정하고 있습니다. Embervale 텔레포트 서비스를 이용해 주셔서 감사합니다.",
 "텔레포트가 완료되었습니다. 저희가 땅을 찾는 동안 움직이지 말아 주세요. 예약할 때는 분명 여기 있었습니다.",
 "땅을 찾았습니다. 안타깝게도 예정보다 일찍 다가오고 있습니다.",
 "빠르게 지나가는 풍경은 신경 쓰지 마세요. 땅과의 약속을 취소하고 더 부드러운 만남을 준비하고 있습니다."},
// pl
{"Usługi Teleportacyjne Embervale",
 "Teleportacja jest już w toku.",
 "Host jest zajęty lub teleportacja jest niedostępna. Spróbuj ponownie za chwilę.",
 "Wciąż sprawdzamy, czy ten serwer ma Embervale Teleport. Spróbuj ponownie za kilka sekund.",
 "Ten serwer nie ma Embervale Teleport, więc teleportacja z mapy jest tu niedostępna. Mod musi być zainstalowany także na serwerze.",
 "Nie udało się ustalić konta tego gracza. Teleportacja anulowana.",
 "Przygotowanie teleportacji trwa dłużej niż zwykle. Twoje zgłoszenie wciąż czeka.",
 "Host nie potwierdził twojego przybycia. Zaczekaj, zanim spróbujesz kolejnej teleportacji.",
 "Przygotowujemy twoją teleportację...",
 "Teleportacja za %u...",
 "Dziękujemy za wybór Usług Teleportacyjnych Embervale.",
 "Zapewniamy, że ten błąd usługi się nie powtórzy. Dziękujemy za wybór Usług Teleportacyjnych Embervale.",
 "Nie udało się ukończyć teleportacji. Sprawdź log Map Teleport u hosta.",
 "Otrzymujesz bezpłatne podwyższenie do luksusowej teleportacji widokowej. Dla twojego bezpieczeństwa wyłączyliśmy twoją lotnię. Miłej podróży i dziękujemy za wybór Usług Teleportacyjnych Embervale.",
 "Wystąpił drobny błąd. Prosimy o chwilę cierpliwości.",
 "Przepraszamy. Zrodzeni z Płomienia najwyraźniej źle reagują na grawitację. To przypadkowe luksusowe podwyższenie się nie powtórzy. Wszystkie kolejne podróże odbędą się w wybranej przez ciebie usłudze. Bezpiecznie sprowadzamy cię na ziemię.",
 "Twój cel podano w metrach. Nasz system przetworzył go w kilometrach. Ciesz się tym bezpłatnym podglądem celu z lotu ptaka.",
 "Nasz dział miar prosi o chwilę cierpliwości.",
 "Znaleźliśmy przecinek. Był na urlopie. Cofnęliśmy mu urlop, więc ten błąd się nie powtórzy. Poprawiamy twój cel. Dziękujemy za wybór Usług Teleportacyjnych Embervale.",
 "Teleportacja zakończona. Prosimy stać spokojnie, gdy szukamy ziemi. Była tu, kiedy robiliśmy rezerwację.",
 "Znaleźliśmy ziemię. Niestety zbliża się przed czasem.",
 "Prosimy nie zwracać uwagi na krajobraz, który szybko przesuwa się obok. Odwołujemy twoje spotkanie z ziemią i organizujemy łagodniejsze zapoznanie."},
// pt (Brazil)
{"Serviços de Teletransporte de Embervale",
 "Já há um teletransporte em andamento.",
 "O anfitrião está ocupado ou o teletransporte está indisponível. Tente novamente em instantes.",
 "Ainda estamos verificando se este servidor usa o Embervale Teleport. Tente novamente em alguns segundos.",
 "Este servidor não usa o Embervale Teleport, então o teletransporte pelo mapa não está disponível aqui. O mod também precisa estar instalado no servidor.",
 "Não foi possível identificar a conta deste jogador. Teletransporte cancelado.",
 "A preparação do teletransporte está demorando mais que o esperado. Sua solicitação ainda está pendente.",
 "O anfitrião não confirmou sua chegada. Aguarde antes de tentar outro teletransporte.",
 "Preparando seu teletransporte...",
 "Teletransporte em %u...",
 "Obrigado por escolher os Serviços de Teletransporte de Embervale.",
 "Garantimos que este erro de serviço não vai se repetir. Obrigado por escolher os Serviços de Teletransporte de Embervale.",
 "Não foi possível concluir o teletransporte. Verifique o log do Map Teleport do anfitrião.",
 "Você recebeu um upgrade para a experiência de teletransporte panorâmico deluxe. Para sua segurança, desativamos o seu planador. Aproveite a viagem, e obrigado por escolher os Serviços de Teletransporte de Embervale.",
 "Houve um pequeno erro. Aguarde um segundo, por favor.",
 "Pedimos desculpas. Os Nascidos da Chama parecem reagir mal à gravidade. Este upgrade deluxe acidental não vai se repetir. Todas as suas próximas viagens usarão o serviço escolhido. Levando você de volta ao chão com segurança.",
 "Seu destino foi informado em metros. Nosso sistema o processou em quilômetros. Aproveite esta prévia aérea gratuita do seu destino.",
 "Nosso departamento de medidas pede um momento de paciência.",
 "Encontramos a vírgula. Ela estava de férias. Cancelamos as férias dela, então este erro não vai se repetir. Corrigindo seu destino agora. Obrigado por escolher os Serviços de Teletransporte de Embervale.",
 "Seu teletransporte foi concluído. Fique parado enquanto localizamos o chão. Ele estava aqui quando fizemos a reserva.",
 "Localizamos o chão. Infelizmente, ele está chegando antes do previsto.",
 "Ignore a paisagem passando rapidamente por você. Estamos cancelando seu compromisso com o chão e providenciando um encontro mais gentil."},
// ru
{"Служба телепортации Embervale",
 "Телепортация уже выполняется.",
 "Хост занят или телепортация недоступна. Попробуйте ещё раз чуть позже.",
 "Всё ещё проверяем, установлен ли на этом сервере Embervale Teleport. Попробуйте ещё раз через несколько секунд.",
 "На этом сервере нет Embervale Teleport, поэтому телепортация по карте здесь недоступна. Мод должен быть установлен и на сервере.",
 "Не удалось определить учётную запись этого игрока. Телепортация отменена.",
 "Подготовка телепортации занимает больше времени, чем ожидалось. Ваш запрос всё ещё в очереди.",
 "Хост не подтвердил ваше прибытие. Подождите, прежде чем пробовать снова.",
 "Готовим вашу телепортацию...",
 "Телепортация через %u...",
 "Спасибо, что выбрали службу телепортации Embervale.",
 "Заверяем вас, что эта ошибка обслуживания больше не повторится. Спасибо, что выбрали службу телепортации Embervale.",
 "Не удалось завершить телепортацию. Проверьте журнал Map Teleport на хосте.",
 "Вам повышен класс до панорамной телепортации делюкс. Для вашей безопасности мы отключили ваш планер. Приятного полёта, и спасибо, что выбрали службу телепортации Embervale.",
 "Произошла небольшая ошибка. Подождите секунду, пожалуйста.",
 "Приносим извинения. Похоже, Пламенорождённые плохо переносят гравитацию. Это случайное повышение класса больше не повторится. Все следующие поездки пройдут по выбранному вами тарифу. Бережно возвращаем вас на землю.",
 "Ваш пункт назначения был указан в метрах. Наша система обработала его в километрах. Наслаждайтесь бесплатным видом на пункт назначения с высоты.",
 "Наш отдел единиц измерения просит минуту терпения.",
 "Мы нашли запятую. Она была в отпуске. Мы отозвали её из отпуска, так что эта ошибка больше не повторится. Исправляем пункт назначения. Спасибо, что выбрали службу телепортации Embervale.",
 "Телепортация завершена. Пожалуйста, не двигайтесь, пока мы ищем землю. Она была здесь, когда мы оформляли заказ.",
 "Мы нашли землю. К сожалению, она приближается раньше графика.",
 "Не обращайте внимания на пейзаж, стремительно проносящийся мимо. Мы отменяем вашу встречу с землёй и готовим более мягкое знакомство."},
// zh-hans
{"Embervale 传送服务",
 "已有传送正在进行中。",
 "主机正忙或传送不可用。请稍后再试。",
 "仍在检查此服务器是否运行 Embervale Teleport。请过几秒再试。",
 "此服务器未运行 Embervale Teleport，因此无法在这里使用地图传送。服务器也需要安装此模组。",
 "无法识别此玩家的账号。传送已取消。",
 "传送准备时间比预期更长。你的请求仍在等待处理。",
 "主机尚未确认你已抵达。请稍候再尝试下一次传送。",
 "正在准备传送...",
 "%u 秒后传送...",
 "感谢选择 Embervale 传送服务。",
 "我们保证此类服务错误不会再次发生。感谢选择 Embervale 传送服务。",
 "传送未能完成。请查看主机的 Map Teleport 日志。",
 "你已升级为豪华观光传送体验。为了你的安全，我们已禁用你的滑翔翼。祝你旅途愉快，感谢选择 Embervale 传送服务。",
 "出了点小差错。请稍等一秒。",
 "非常抱歉。焰生者似乎不太适应重力。这次意外的豪华升级不会再发生。今后的所有旅程都将使用你选择的服务。现在正安全地送你回到地面。",
 "你的目的地是以米为单位输入的，我们的系统却按公里处理了。请享受这次免费的目的地空中预览。",
 "我们的计量部门请你耐心稍等片刻。",
 "我们找到了小数点。它去度假了。我们已取消它的假期，所以此错误不会再发生。正在修正你的目的地。感谢选择 Embervale 传送服务。",
 "传送已完成。请保持不动，我们正在寻找地面。预订时它明明还在这里。",
 "我们找到地面了。不幸的是，它正提前向你靠近。",
 "请忽略飞速掠过的风景。我们正在取消你与地面的约会，并安排一次更温柔的见面。"},
// zh-hant
{"Embervale 傳送服務",
 "已有傳送正在進行中。",
 "主機正忙或無法傳送。請稍後再試。",
 "仍在檢查此伺服器是否執行 Embervale Teleport。請過幾秒再試。",
 "此伺服器未執行 Embervale Teleport，因此無法在這裡使用地圖傳送。伺服器也需要安裝此模組。",
 "無法識別此玩家的帳號。傳送已取消。",
 "傳送準備時間比預期更長。你的請求仍在等待處理。",
 "主機尚未確認你已抵達。請稍候再嘗試下一次傳送。",
 "正在準備傳送...",
 "%u 秒後傳送...",
 "感謝選擇 Embervale 傳送服務。",
 "我們保證此類服務錯誤不會再次發生。感謝選擇 Embervale 傳送服務。",
 "傳送未能完成。請查看主機的 Map Teleport 記錄檔。",
 "你已升級為豪華觀光傳送體驗。為了你的安全，我們已停用你的滑翔翼。祝你旅途愉快，感謝選擇 Embervale 傳送服務。",
 "出了點小差錯。請稍等一秒。",
 "非常抱歉。焰生者似乎不太適應重力。這次意外的豪華升級不會再發生。今後的所有旅程都將使用你選擇的服務。現在正安全地送你回到地面。",
 "你的目的地是以公尺為單位輸入的，我們的系統卻按公里處理了。請享受這次免費的目的地空中預覽。",
 "我們的計量部門請你耐心稍候片刻。",
 "我們找到了小數點。它去度假了。我們已取消它的假期，所以此錯誤不會再發生。正在修正你的目的地。感謝選擇 Embervale 傳送服務。",
 "傳送已完成。請保持不動，我們正在尋找地面。預訂時它明明還在這裡。",
 "我們找到地面了。不幸的是，它正提前向你靠近。",
 "請忽略飛速掠過的風景。我們正在取消你與地面的約會，並安排一次更溫柔的見面。"},
// tr
{"Embervale Işınlanma Hizmetleri",
 "Zaten devam eden bir ışınlanma var.",
 "Sunucu meşgul ya da ışınlanma şu anda kullanılamıyor. Lütfen birazdan tekrar dene.",
 "Bu sunucunun Embervale Teleport kullanıp kullanmadığı hâlâ kontrol ediliyor. Lütfen birkaç saniye sonra tekrar dene.",
 "Bu sunucuda Embervale Teleport yok, bu yüzden burada haritadan ışınlanma kullanılamaz. Modun sunucuya da kurulması gerekir.",
 "Bu oyuncunun hesabı belirlenemedi. Işınlanma iptal edildi.",
 "Işınlanma hazırlığı beklenenden uzun sürüyor. İsteğin hâlâ beklemede.",
 "Sunucu varışını onaylamadı. Yeni bir ışınlanma denemeden önce lütfen bekle.",
 "Işınlanman hazırlanıyor...",
 "Işınlanma %u saniye içinde...",
 "Embervale Işınlanma Hizmetleri'ni seçtiğin için teşekkür ederiz.",
 "Bu hizmet hatasının bir daha yaşanmayacağına seni temin ederiz. Embervale Işınlanma Hizmetleri'ni seçtiğin için teşekkür ederiz.",
 "Işınlanma tamamlanamadı. Sunucudaki Map Teleport günlüğüne bak.",
 "Lüks manzaralı ışınlanma deneyimine yükseltildin. Güvenliğin için planörünü devre dışı bıraktık. Yolculuğun tadını çıkar ve Embervale Işınlanma Hizmetleri'ni seçtiğin için teşekkür ederiz.",
 "Küçük bir hata oldu. Lütfen bir saniye bekle.",
 "Özür dileriz. Alevden Doğanlar yerçekimine pek iyi tepki vermiyor gibi görünüyor. Bu kazara lüks yükseltme bir daha yaşanmayacak. Sonraki tüm yolculukların seçtiğin hizmetle yapılacak. Seni şimdi güvenle yere indiriyoruz.",
 "Varış noktan metre olarak girildi. Sistemimiz onu kilometre olarak işledi. Varış noktanın bu ücretsiz havadan önizlemesinin tadını çıkar.",
 "Ölçü birimleri departmanımız bir dakikalık sabrını rica ediyor.",
 "Virgülü bulduk. Tatildeydi. İznini iptal ettik, bu yüzden bu hata bir daha yaşanmayacak. Varış noktan şimdi düzeltiliyor. Embervale Işınlanma Hizmetleri'ni seçtiğin için teşekkür ederiz.",
 "Işınlanman tamamlandı. Biz zemini bulurken lütfen kıpırdama. Rezervasyonu yaptığımızda buradaydı.",
 "Zemini bulduk. Ne yazık ki planlanandan erken yaklaşıyor.",
 "Lütfen hızla yanından geçen manzarayı dikkate alma. Zeminle olan randevunu iptal ediyor ve daha nazik bir tanışma ayarlıyoruz."},
// uk
{"Служба телепортації Embervale",
 "Телепортація вже триває.",
 "Хост зайнятий або телепортація недоступна. Спробуйте ще раз трохи згодом.",
 "Ще перевіряємо, чи встановлено на цьому сервері Embervale Teleport. Спробуйте ще раз за кілька секунд.",
 "На цьому сервері немає Embervale Teleport, тому телепортація з мапи тут недоступна. Мод має бути встановлений і на сервері.",
 "Не вдалося визначити обліковий запис цього гравця. Телепортацію скасовано.",
 "Підготовка телепортації триває довше, ніж очікувалося. Ваш запит досі в черзі.",
 "Хост не підтвердив ваше прибуття. Зачекайте, перш ніж пробувати знову.",
 "Готуємо вашу телепортацію...",
 "Телепортація через %u...",
 "Дякуємо, що обрали службу телепортації Embervale.",
 "Запевняємо вас, що ця помилка обслуговування більше не повториться. Дякуємо, що обрали службу телепортації Embervale.",
 "Не вдалося завершити телепортацію. Перевірте журнал Map Teleport на хості.",
 "Вам підвищено клас до панорамної телепортації делюкс. Задля вашої безпеки ми вимкнули ваш планер. Приємної подорожі, і дякуємо, що обрали службу телепортації Embervale.",
 "Сталася невеличка помилка. Зачекайте секунду, будь ласка.",
 "Перепрошуємо. Схоже, Полум'янароджені погано переносять гравітацію. Це випадкове підвищення класу більше не повториться. Усі наступні подорожі пройдуть за обраним вами тарифом. Обережно повертаємо вас на землю.",
 "Ваш пункт призначення вказали в метрах. Наша система обробила його в кілометрах. Насолоджуйтеся безкоштовним видом на пункт призначення з висоти.",
 "Наш відділ одиниць виміру просить хвилинку терпіння.",
 "Ми знайшли кому. Вона була у відпустці. Ми відкликали її з відпустки, тож ця помилка більше не повториться. Виправляємо пункт призначення. Дякуємо, що обрали службу телепортації Embervale.",
 "Телепортацію завершено. Будь ласка, не рухайтеся, поки ми шукаємо землю. Вона була тут, коли ми оформлювали замовлення.",
 "Ми знайшли землю. На жаль, вона наближається раніше за графік.",
 "Не зважайте на краєвид, що стрімко проноситься повз. Ми скасовуємо вашу зустріч із землею і готуємо м'якше знайомство."},
// th
{"บริการเทเลพอร์ต Embervale",
 "มีการเทเลพอร์ตกำลังดำเนินอยู่แล้ว",
 "โฮสต์ไม่ว่าง หรือไม่สามารถเทเลพอร์ตได้ในขณะนี้ โปรดลองอีกครั้งในอีกสักครู่",
 "กำลังตรวจสอบว่าเซิร์ฟเวอร์นี้ใช้ Embervale Teleport หรือไม่ โปรดลองอีกครั้งในอีกไม่กี่วินาที",
 "เซิร์ฟเวอร์นี้ไม่ได้ใช้ Embervale Teleport จึงไม่สามารถเทเลพอร์ตจากแผนที่ได้ที่นี่ ต้องติดตั้งม็อดบนเซิร์ฟเวอร์ด้วย",
 "ไม่สามารถระบุบัญชีของผู้เล่นนี้ได้ ยกเลิกการเทเลพอร์ตแล้ว",
 "การเตรียมเทเลพอร์ตใช้เวลานานกว่าที่คาดไว้ คำขอของคุณยังรอดำเนินการอยู่",
 "โฮสต์ยังไม่ยืนยันว่าคุณมาถึงแล้ว โปรดรอสักครู่ก่อนเทเลพอร์ตครั้งต่อไป",
 "กำลังเตรียมการเทเลพอร์ต...",
 "เทเลพอร์ตใน %u...",
 "ขอบคุณที่เลือกใช้บริการเทเลพอร์ต Embervale",
 "เราขอรับรองว่าข้อผิดพลาดของบริการนี้จะไม่เกิดขึ้นอีก ขอบคุณที่เลือกใช้บริการเทเลพอร์ต Embervale",
 "ไม่สามารถเทเลพอร์ตให้เสร็จสิ้นได้ โปรดตรวจสอบบันทึก Map Teleport ของโฮสต์",
 "คุณได้รับการอัปเกรดเป็นประสบการณ์เทเลพอร์ตชมวิวระดับดีลักซ์ เพื่อความปลอดภัยของคุณ เราได้ปิดเครื่องร่อนของคุณไว้ ขอให้สนุกกับการเดินทาง และขอบคุณที่เลือกใช้บริการเทเลพอร์ต Embervale",
 "เกิดข้อผิดพลาดเล็กน้อย โปรดรอสักครู่",
 "เราขออภัย ดูเหมือนผู้กำเนิดจากเปลวไฟจะไม่ค่อยถูกกับแรงโน้มถ่วง การอัปเกรดดีลักซ์โดยไม่ตั้งใจนี้จะไม่เกิดขึ้นอีก การเดินทางครั้งต่อไปทั้งหมดจะใช้บริการที่คุณเลือก กำลังพาคุณกลับลงสู่พื้นอย่างปลอดภัย",
 "จุดหมายของคุณถูกป้อนเป็นเมตร แต่ระบบของเราประมวลผลเป็นกิโลเมตร ขอให้เพลิดเพลินกับการชมจุดหมายจากมุมสูงฟรี",
 "แผนกหน่วยวัดของเราขอความกรุณารอสักครู่",
 "เราพบจุดทศนิยมแล้ว มันไปพักร้อนอยู่ เราได้ยกเลิกวันลาของมันแล้ว ข้อผิดพลาดนี้จึงจะไม่เกิดขึ้นอีก กำลังแก้ไขจุดหมายของคุณ ขอบคุณที่เลือกใช้บริการเทเลพอร์ต Embervale",
 "เทเลพอร์ตเสร็จสมบูรณ์ โปรดอยู่นิ่ง ๆ ระหว่างที่เราหาพื้นดิน ตอนจองมันยังอยู่ตรงนี้",
 "เราพบพื้นดินแล้ว น่าเสียดายที่มันกำลังเข้ามาใกล้เร็วกว่ากำหนด",
 "โปรดอย่าสนใจทิวทัศน์ที่ผ่านไปอย่างรวดเร็ว เรากำลังยกเลิกนัดของคุณกับพื้นดิน และจัดการพบกันที่นุ่มนวลกว่านี้"},
};

inline const char* Get(Lang lang,Id id) {
    auto row=size_t(lang)<size_t(Lang::count)?size_t(lang):0;
    return id>=0&&id<IdCount?kTexts[row][id]:"";
}
// The scenario's opening, problem or correction line (part 0, 1, 2).
inline const char* ScenarioLine(Lang lang,unsigned scenario,unsigned part) {
    return Get(lang,Id(Scenario0Opening+(scenario%3)*3+(part%3)));
}

// "auto" (or anything unknown) is -1; otherwise the language with that code ("de", "zh-hans").
inline int ParseCode(std::string_view code) {
    std::string lower;
    for(char c:code) if(c!=' '&&c!='\t')lower+=char(c>='A'&&c<='Z'?c-'A'+'a':c);
    if(lower=="zh"||lower=="zh-cn"||lower=="zh_cn"||lower=="zh_hans")lower="zh-hans";
    if(lower=="zh-tw"||lower=="zh_tw"||lower=="zh_hant")lower="zh-hant";
    if(lower=="pt-br"||lower=="pt_br")lower="pt";
    for(size_t i=0;i<size_t(Lang::count);++i) if(lower==kCodes[i])return int(i);
    return -1;
}

// The game's language from its own word for "Legendary" (the rarity), as Find tells it.
// False when the text is empty or unknown; the caller keeps its previous language then.
inline bool Detect(std::string_view legendary,Lang& out) {
    auto has=[&](const char* part){return legendary.find(part)!=std::string_view::npos;};
    auto starts=[&](const char* prefix) {
        size_t n=strlen(prefix);if(legendary.size()<n)return false;
        for(size_t i=0;i<n;++i){char a=legendary[i];if(a>='A'&&a<='Z')a=char(a-'A'+'a');if(a!=prefix[i])return false;}
        return true;
    };
    if(legendary.empty())return false;
    if(starts("legendary"))out=Lang::en;
    else if(starts("legendär")||starts("legendÄr"))out=Lang::de;
    else if(starts("légend")||starts("lÉgend"))out=Lang::fr;
    else if(starts("legendari"))out=Lang::es;
    else if(starts("leggend"))out=Lang::it;
    else if(starts("lend"))out=Lang::pt;
    else if(starts("legendarn"))out=Lang::pl;
    else if(starts("efsane"))out=Lang::tr;
    else if(has("ы")||has("Ы"))out=Lang::ru;
    else if(has("Леген")||has("ЛЕГЕН")||has("леген"))out=Lang::uk;
    else if(has("전설"))out=Lang::ko;
    else if(has("レジェ")||has("伝説"))out=Lang::ja;
    else if(has("传")||has("说"))out=Lang::zhHans;
    else if(has("傳")||has("說"))out=Lang::zhHant;
    else if(legendary.size()>=3&&uint8_t(legendary[0])==0xE0&&(uint8_t(legendary[1])==0xB8||uint8_t(legendary[1])==0xB9))out=Lang::th;
    else return false;
    return true;
}

} // namespace teleport::text

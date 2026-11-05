
import matplotlib.pyplot as plt
import os

# Tempos de execução medidos no Item 1 (ms)
C = {
    "T1": 277.759288,
    "T2": 204.555718,
    "T3": 262.851810
}

# Períodos e deadlines (ms)
T = {
    "T1": 100,
    "T2": 200,
    "T3": 300
}

HORIZON = 600
SIMULATION_END = 3000

def simulate(priority_order):
    """Simula tarefas periódicas com prioridades fixas e preempção."""

    priority = {
        task: index
        for index, task in enumerate(priority_order)
    }

    jobs = []
    segments = []
    next_release = {task: 0 for task in C}
    job_number = {task: 0 for task in C}

    time = 0.0

    while time < SIMULATION_END:
        # Libertar todas as tarefas cujo instante de ativação chegou
        for task in C:
            while next_release[task] <= time + 1e-9:
                job_number[task] += 1

                jobs.append({
                    "task": task,
                    "number": job_number[task],
                    "release": next_release[task],
                    "deadline": next_release[task] + T[task],
                    "remaining": C[task],
                    "completion": None
                })

                next_release[task] += T[task]

        # Selecionar a tarefa pronta com maior prioridade
        ready = [job for job in jobs if job["remaining"] > 1e-9]

        if not ready:
            time = min(next_release.values())
            continue

        current = min(
            ready,
            key=lambda job: (
                priority[job["task"]],
                job["release"],
                job["number"]
            )
        )

        # Próxima ativação ou conclusão da tarefa atual
        next_activation = min(next_release.values())
        duration = min(
            current["remaining"],
            next_activation - time
        )

        if duration <= 1e-9:
            continue

        end = time + duration

        segments.append({
            "task": current["task"],
            "start": time,
            "end": end
        })

        current["remaining"] -= duration
        time = end

        if current["remaining"] <= 1e-9:
            current["remaining"] = 0
            current["completion"] = time

    return segments, jobs


def create_gantt(priority_order, filename, title):
    segments, jobs = simulate(priority_order)

    fig, ax = plt.subplots(figsize=(13, 4.5))

    colors = {
        "T1": "steelblue",
        "T2": "darkorange",
        "T3": "seagreen"
    }

    positions = {"T1": 3, "T2": 2, "T3": 1}

    # Desenhar intervalos de execução
    for segment in segments:
        start = segment["start"]
        end = segment["end"]

        if start >= HORIZON:
            continue

        end = min(end, HORIZON)

        ax.barh(
            positions[segment["task"]],
            end - start,
            left=start,
            height=0.55,
            color=colors[segment["task"]],
            edgecolor="black"
        )

    # Marcar deadlines e assinalar os que foram perdidos
    for job in jobs:
        deadline = job["deadline"]

        if deadline > HORIZON:
            continue

        y = positions[job["task"]]

        ax.plot(
            deadline, y + 0.34,
            marker="|", color="black",
            markersize=9
        )

        missed = (
            job["completion"] is None
            or job["completion"] > deadline + 1e-9
        )

        if missed:
            ax.plot(
                deadline, y + 0.48,
                marker="x", color="red",
                markersize=7
            )

    ax.set_yticks([1, 2, 3])
    ax.set_yticklabels(["T3", "T2", "T1"])
    ax.set_xlim(0, HORIZON)
    ax.set_xticks(range(0, HORIZON + 1, 50))
    ax.set_xlabel("Tempo (ms)")
    ax.set_ylabel("Tarefas")
    ax.set_title(title)
    ax.grid(axis="x", linestyle="--", alpha=0.5)

    fig.tight_layout()
    fig.savefig(filename, dpi=200)
    plt.close(fig)

    print(f"Diagrama guardado: {filename}")


os.makedirs("figures", exist_ok=True)

create_gantt(
    ["T1", "T2", "T3"],
    "figures/gantt_rmpo.png",
    "Escalonamento RMPO (T1 > T2 > T3)"
)

create_gantt(
    ["T3", "T2", "T1"],
    "figures/gantt_inverso.png",
    "Escalonamento inverso (T3 > T2 > T1)"
)

print("\nDiagramas criados com sucesso.")
